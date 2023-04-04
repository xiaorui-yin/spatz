// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Matheus Cavalcante <matheusd@iis.ee.ethz.ch>

`include "axi/assign.svh"
`include "axi/typedef.svh"
`include "common_cells/assertions.svh"
`include "common_cells/registers.svh"

/// Spatz many-core cluster with improved TCDM interconnect.
/// Spatz Cluster Top-Level.
module spatz_cluster import spatz_pkg::*; #(
    /// Width of physical address.
    parameter int   unsigned        AxiAddrWidth       = 48,
    /// Width of AXI port.
    parameter int   unsigned        AxiDataWidth       = 512,
    /// AXI: id width in.
    parameter int   unsigned        AxiIdWidth         = 2,
    /// AXI: user width.
    parameter int   unsigned        AxiUserWidth       = 1,
    /// Address from which to fetch the first instructions.
    parameter logic          [31:0] BootAddr           = 32'h0,
    /// The total amount of cores.
    parameter int   unsigned        NrCores            = 8,
    /// Data/TCDM memory depth per cut (in words).
    parameter int   unsigned        TCDMDepth          = 1024,
    /// Zero memory address region size (in kB).
    parameter int   unsigned        ZeroMemorySize     = 64,
    /// Cluster peripheral address region size (in kB).
    parameter int   unsigned        ClusterPeriphSize  = 64,
    /// Number of TCDM Banks.
    parameter int   unsigned        NrBanks            = 2 * NrCores,
    /// Size of DMA AXI buffer.
    parameter int   unsigned        DMAAxiReqFifoDepth = 3,
    /// Size of DMA request fifo.
    parameter int   unsigned        DMAReqFifoDepth    = 3,
    /// Width of a single icache line.
    parameter int   unsigned        ICacheLineWidth    = '{default: 0},
    /// Number of icache lines per set.
    parameter int   unsigned        ICacheLineCount    = '{default: 0},
    /// Number of icache sets.
    parameter int   unsigned        ICacheSets         = '{default: 0},
    /// # Core-global parameters
    /// FPU configuration.
    parameter fpnew_pkg::fpu_implementation_t FPUImplementation [NrCores] = '{default: fpnew_pkg::fpu_implementation_t'(0)},
    /// # Per-core parameters
    /// Per-core integer outstanding loads
    parameter int unsigned NumIntOutstandingLoads [NrCores] = '{default: 0},
    /// Per-core integer outstanding memory operations (load and stores)
    parameter int unsigned NumIntOutstandingMem   [NrCores] = '{default: 0},
    /// Per-core floating-point outstanding loads
    parameter int unsigned NumFPOutstandingLoads  [NrCores] = '{default: 0},
    /// Per-core floating-point outstanding memory operations (load and stores)
    parameter int unsigned NumFPOutstandingMem    [NrCores] = '{default: 0},
    /// ## Timing Tuning Parameters
    /// Insert Pipeline registers into off-loading path (request)
    parameter bit          RegisterOffloadReq               = 1'b0,
    /// Insert Pipeline registers into off-loading path (response)
    parameter bit          RegisterOffloadRsp               = 1'b0,
    /// Insert Pipeline registers into data memory path (request)
    parameter bit          RegisterCoreReq                  = 1'b0,
    /// Insert Pipeline registers into data memory path (response)
    parameter bit          RegisterCoreRsp                  = 1'b0,
    /// Insert Pipeline registers after each memory cut
    parameter bit          RegisterTCDMCuts                 = 1'b0,
    /// Decouple external AXI plug
    parameter bit          RegisterExt                      = 1'b0,
    parameter axi_pkg::xbar_latency_e XbarLatency = axi_pkg::CUT_ALL_PORTS,
    /// Outstanding transactions on the AXI network
    parameter int  unsigned MaxMstTrans        = 4,
    parameter int  unsigned MaxSlvTrans        = 4,
    /// # Interface
    /// AXI Ports
    parameter type          axi_req_t          = logic,
    parameter type          axi_resp_t         = logic,
    // Memory configuration input types; these vary depending on implementation.
    parameter type          sram_cfg_t         = logic,
    parameter type          sram_cfgs_t        = logic,
    // Memory latency parameter. Most of the memories have a read latency of 1. In
    // case you have memory macros which are pipelined you want to adjust this
    // value here. This only applies to the TCDM. The instruction cache macros will break!
    // In case you are using the `RegisterTCDMCuts` feature this adds an
    // additional cycle latency, which is taken into account here.
    parameter int  unsigned MemoryMacroLatency = 1 + RegisterTCDMCuts
  ) (
    /// System clock.
    input  logic                          clk_i,
    /// Asynchronous active high reset. This signal is assumed to be _async_.
    input  logic                          rst_ni,
    /// Per-core debug request signal. Asserting this signals puts the
    /// corresponding core into debug mode. This signal is assumed to be _async_.
    input  logic       [NrCores-1:0]      debug_req_i,
    /// Machine external interrupt pending. Usually those interrupts come from a
    /// platform-level interrupt controller. This signal is assumed to be _async_.
    input  logic       [NrCores-1:0]      meip_i,
    /// Machine timer interrupt pending. Usually those interrupts come from a
    /// core-local interrupt controller such as a timer/RTC. This signal is
    /// assumed to be _async_.
    input  logic       [NrCores-1:0]      mtip_i,
    /// Core software interrupt pending. Usually those interrupts come from
    /// another core to facilitate inter-processor-interrupts. This signal is
    /// assumed to be _async_.
    input  logic       [NrCores-1:0]      msip_i,
    /// First hartid of the cluster. Cores of a cluster are monotonically
    /// increasing without a gap, i.e., a cluster with 8 cores and a
    /// `hart_base_id_i` of 5 get the hartids 5 - 12.
    input  logic       [9:0]              hart_base_id_i,
    /// Base address of cluster. TCDM and cluster peripheral location are derived from
    /// it. This signal is pseudo-static.
    input  logic       [AxiAddrWidth-1:0] cluster_base_addr_i,
    /// Configuration inputs for the memory cuts used in implementation.
    /// These signals are pseudo-static.
    input  sram_cfgs_t                    sram_cfgs_i,
    /// Per-cluster probe on the cluster status. Can be written by the cores to indicate
    /// to the overall system that the cluster is executing something.
    output logic                          cluster_probe_o,
    /// Per-cluster probe on the EOC status. Indicates the end of the execution.
    output logic                          eoc_o,
    /// AXI Core cluster in-port.
    input  axi_req_t                      axi_in_req_i,
    output axi_resp_t                     axi_in_resp_o,
    /// AXI Core cluster out-port.
    output axi_req_t                      axi_out_req_o,
    input  axi_resp_t                     axi_out_resp_i
  );
  // ---------
  // Imports
  // ---------
  import snitch_pkg::*;
  import snitch_icache_pkg::icache_events_t;

  // ---------
  // Constants
  // ---------
  /// Minimum width to hold the core number.
  localparam int unsigned CoreIDWidth       = cf_math_pkg::idx_width(NrCores);
  localparam int unsigned TCDMMemAddrWidth  = $clog2(TCDMDepth);
  localparam int unsigned TCDMSize          = NrBanks * TCDMDepth * (DataWidth/8);
  localparam int unsigned TCDMAddrWidth     = $clog2(TCDMSize);
  localparam int unsigned BanksPerSuperBank = AxiDataWidth / DataWidth;
  localparam int unsigned NrSuperBanks      = NrBanks / BanksPerSuperBank;

  function automatic int unsigned get_tcdm_ports(int unsigned core);
    return spatz_pkg::N_FU + 1;
  endfunction

  function automatic int unsigned get_tcdm_port_offs(int unsigned core_idx);
    automatic int n = 0;
    for (int i = 0; i < core_idx; i++) n += get_tcdm_ports(i);
    return n;
  endfunction

  localparam int unsigned NrTCDMPortsCores = get_tcdm_port_offs(NrCores);
  localparam int unsigned NumTCDMIn        = NrTCDMPortsCores + 1;

  localparam logic [AxiAddrWidth-1:0] TCDMMask = ~(TCDMSize-1);

  // We have a single hive
  localparam int unsigned NrHives = 1;

  // Core Requests, SoC Request, DMA, Instruction cache.
  localparam int unsigned NrMasters         = 3 + NrHives;
  localparam int unsigned AxiIdWidthCluster = AxiIdWidth - $clog2(NrMasters);

  localparam int unsigned NrSlaves = 3;
  localparam int unsigned NrRules  = NrSlaves - 1;

  // AXI Configuration
  localparam axi_pkg::xbar_cfg_t ClusterXbarCfg = '{
    NoSlvPorts        : NrMasters,
    NoMstPorts        : NrSlaves,
    MaxMstTrans       : MaxMstTrans,
    MaxSlvTrans       : MaxSlvTrans,
    FallThrough       : 1'b0,
    LatencyMode       : XbarLatency,
    PipelineStages    : 0,
    AxiIdWidthSlvPorts: AxiIdWidth,
    AxiIdUsedSlvPorts : AxiIdWidth,
    UniqueIds         : 1'b0,
    AxiAddrWidth      : AxiAddrWidth,
    AxiDataWidth      : AxiDataWidth,
    NoAddrRules       : NrRules
  };

  // --------
  // Typedefs
  // --------
  typedef logic [AxiAddrWidth-1:0] axi_addr_t;
  typedef logic [AxiDataWidth-1:0] axi_data_t;
  typedef logic [AxiDataWidth/8-1:0] axi_strb_t;
  typedef logic [AxiIdWidth-1:0] axi_id_mst_t;
  typedef logic [AxiIdWidthCluster-1:0] axi_id_slv_t;
  typedef logic [AxiUserWidth-1:0] axi_user_t;

  typedef logic [TCDMMemAddrWidth-1:0] tcdm_mem_addr_t;
  typedef logic [TCDMAddrWidth-1:0] tcdm_addr_t;
  typedef logic [DataWidth-1:0] tcdm_data_t;
  typedef logic [DataWidth/8-1:0] tcdm_strb_t;
  typedef struct packed {
    logic [CoreIDWidth-1:0] core_id;
    bit is_core;
  } tcdm_user_t;

  // Regbus peripherals.
  `AXI_TYPEDEF_ALL(axi_mst, axi_addr_t, axi_id_mst_t, axi_data_t, axi_strb_t, axi_user_t)
  `AXI_TYPEDEF_ALL(axi_slv, axi_addr_t, axi_id_slv_t, axi_data_t, axi_strb_t, axi_user_t)

  `REQRSP_TYPEDEF_ALL(reqrsp, axi_addr_t, tcdm_data_t, tcdm_strb_t)

  `MEM_TYPEDEF_ALL(mem, tcdm_mem_addr_t, tcdm_data_t, tcdm_strb_t, tcdm_user_t)
  `MEM_TYPEDEF_ALL(mem_dma, tcdm_mem_addr_t, axi_data_t, axi_strb_t, logic)

  `TCDM_TYPEDEF_ALL(tcdm, tcdm_addr_t, tcdm_data_t, tcdm_strb_t, tcdm_user_t)
  `TCDM_TYPEDEF_ALL(tcdm_dma, tcdm_addr_t, axi_data_t, axi_strb_t, logic)

  `REG_BUS_TYPEDEF_REQ(reg_req_t, axi_addr_t, axi_data_t, axi_strb_t)
  `REG_BUS_TYPEDEF_RSP(reg_rsp_t, axi_data_t)

  // Event counter increments for the TCDM.
  typedef struct packed {
    /// Number requests going in
    logic [$clog2(NrTCDMPortsCores):0] inc_accessed;
    /// Number of requests stalled due to congestion
    logic [$clog2(NrTCDMPortsCores):0] inc_congested;
  } tcdm_events_t;

  // Event counter increments for DMA.
  typedef struct packed {
    logic aw_stall, ar_stall, r_stall, w_stall,
    buf_w_stall, buf_r_stall;
    logic aw_valid, aw_ready, aw_done, aw_bw;
    logic ar_valid, ar_ready, ar_done, ar_bw;
    logic r_valid, r_ready, r_done, r_bw;
    logic w_valid, w_ready, w_done, w_bw;
    logic b_valid, b_ready, b_done;
    logic dma_busy;
    axi_pkg::len_t aw_len, ar_len;
    axi_pkg::size_t aw_size, ar_size;
    logic [$clog2(AxiDataWidth/8):0] num_bytes_written;
  } dma_events_t;

  typedef struct packed {
    int unsigned idx;
    axi_addr_t start_addr;
    axi_addr_t end_addr;
  } xbar_rule_t;

  typedef struct packed {
    acc_addr_e addr;
    logic [4:0] id;
    logic [31:0] data_op;
    axi_data_t data_arga;
    axi_data_t data_argb;
    axi_addr_t data_argc;
  } acc_req_t;

  typedef struct packed {
    logic [4:0] id;
    logic error;
    axi_data_t data;
  } acc_resp_t;

  `SNITCH_VM_TYPEDEF(AxiAddrWidth)

  typedef struct packed {
    // Slow domain.
    logic flush_i_valid;
    axi_addr_t inst_addr;
    logic inst_cacheable;
    logic inst_valid;
    // Fast domain.
    acc_req_t acc_req;
    logic acc_qvalid;
    logic acc_pready;
    // Slow domain.
    logic [1:0] ptw_valid;
    va_t [1:0] ptw_va;
    pa_t [1:0] ptw_ppn;
  } hive_req_t;

  typedef struct packed {
    // Slow domain.
    logic flush_i_ready;
    logic [31:0] inst_data;
    logic inst_ready;
    logic inst_error;
    // Fast domain.
    logic acc_qready;
    acc_resp_t acc_resp;
    logic acc_pvalid;
    // Slow domain.
    logic [1:0] ptw_ready;
    l0_pte_t [1:0] ptw_pte;
    logic [1:0] ptw_is_4mega;
  } hive_rsp_t;

  // -----------
  // Assignments
  // -----------
  // Calculate start and end address of TCDM based on the `cluster_base_addr_i`.
  axi_addr_t tcdm_start_address, tcdm_end_address;
  assign tcdm_start_address = (cluster_base_addr_i & TCDMMask);
  assign tcdm_end_address   = (tcdm_start_address + TCDMSize) & TCDMMask;

  axi_addr_t cluster_periph_start_address, cluster_periph_end_address;
  assign cluster_periph_start_address = tcdm_end_address;
  assign cluster_periph_end_address   = tcdm_end_address + ClusterPeriphSize * 1024;

  axi_addr_t zero_mem_start_address, zero_mem_end_address;
  assign zero_mem_start_address = cluster_periph_end_address;
  assign zero_mem_end_address   = cluster_periph_end_address + ZeroMemorySize * 1024;

  // ----------------
  // Wire Definitions
  // ----------------
  // 1. AXI
  axi_slv_req_t  [NrSlaves-1:0] axi_slv_req;
  axi_slv_resp_t [NrSlaves-1:0] axi_slv_rsp;

  axi_mst_req_t  [NrMasters-1:0] axi_mst_req;
  axi_mst_resp_t [NrMasters-1:0] axi_mst_rsp;

  // 2. Memory Subsystem (Banks)
  mem_req_t [NrSuperBanks-1:0][BanksPerSuperBank-1:0] ic_req;
  mem_rsp_t [NrSuperBanks-1:0][BanksPerSuperBank-1:0] ic_rsp;

  mem_dma_req_t [NrSuperBanks-1:0] sb_dma_req;
  mem_dma_rsp_t [NrSuperBanks-1:0] sb_dma_rsp;

  // 3. Memory Subsystem (Interconnect)
  tcdm_dma_req_t ext_dma_req;
  tcdm_dma_rsp_t ext_dma_rsp;

  // AXI Ports into TCDM (from SoC).
  tcdm_req_t axi_soc_req;
  tcdm_rsp_t axi_soc_rsp;

  tcdm_req_t [NrTCDMPortsCores-1:0] tcdm_req;
  tcdm_rsp_t [NrTCDMPortsCores-1:0] tcdm_rsp;

  core_events_t   [NrCores-1:0] core_events;
  tcdm_events_t                 tcdm_events;
  dma_events_t                  dma_events;
  icache_events_t [NrCores-1:0] icache_events;

  // 4. Memory Subsystem (Core side).
  reqrsp_req_t [NrCores-1:0] core_req, filtered_core_req;
  reqrsp_rsp_t [NrCores-1:0] core_rsp, filtered_core_rsp;
  reqrsp_req_t [NrHives-1:0] ptw_req;
  reqrsp_rsp_t [NrHives-1:0] ptw_rsp;

  // 5. Peripheral Subsystem
  reg_req_t reg_req;
  reg_rsp_t reg_rsp;

  // 5. Misc. Wires.
  logic               icache_prefetch_enable;
  logic [NrCores-1:0] cl_interrupt;

  // -------------
  // DMA Subsystem
  // -------------
  // Optionally decouple the external wide AXI master port.
  axi_cut #(
    .Bypass     (!RegisterExt     ),
    .aw_chan_t  (axi_slv_aw_chan_t),
    .w_chan_t   (axi_slv_w_chan_t ),
    .b_chan_t   (axi_slv_b_chan_t ),
    .ar_chan_t  (axi_slv_ar_chan_t),
    .r_chan_t   (axi_slv_r_chan_t ),
    .axi_req_t  (axi_slv_req_t    ),
    .axi_resp_t (axi_slv_resp_t   )
  ) i_cut_ext_wide_out (
    .clk_i      (clk_i                 ),
    .rst_ni     (rst_ni                ),
    .slv_req_i  (axi_slv_req[SoCDMAOut]),
    .slv_resp_o (axi_slv_rsp[SoCDMAOut]),
    .mst_req_o  (axi_out_req_o         ),
    .mst_resp_i (axi_out_resp_i        )
  );

  axi_cut #(
    .Bypass     (!RegisterExt     ),
    .aw_chan_t  (axi_mst_aw_chan_t),
    .w_chan_t   (axi_mst_w_chan_t ),
    .b_chan_t   (axi_mst_b_chan_t ),
    .ar_chan_t  (axi_mst_ar_chan_t),
    .r_chan_t   (axi_mst_r_chan_t ),
    .axi_req_t  (axi_mst_req_t    ),
    .axi_resp_t (axi_mst_resp_t   )
  ) i_cut_ext_wide_in (
    .clk_i      (clk_i                ),
    .rst_ni     (rst_ni               ),
    .slv_req_i  (axi_in_req_i         ),
    .slv_resp_o (axi_in_resp_o        ),
    .mst_req_o  (axi_mst_req[SoCDMAIn]),
    .mst_resp_i (axi_mst_rsp[SoCDMAIn])
  );

  logic       [ClusterXbarCfg.NoSlvPorts-1:0][$clog2(ClusterXbarCfg.NoMstPorts)-1:0] xbar_default_port;
  xbar_rule_t [ClusterXbarCfg.NoAddrRules-1:0]                                       xbar_rule;

  assign xbar_default_port = '{default: SoCDMAOut};
  assign xbar_rule         = '{
    '{
      idx       : TCDMDMA,
      start_addr: tcdm_start_address,
      end_addr  : tcdm_end_address
    },
    '{
      idx       : ZeroMemory,
      start_addr: zero_mem_start_address,
      end_addr  : zero_mem_end_address
    }
  };
  localparam bit [ClusterXbarCfg.NoSlvPorts-1:0] DMAEnableDefaultMstPort = '1;
  axi_xbar #(
    .Cfg           (ClusterXbarCfg   ),
    .ATOPs         (0                ),
    .slv_aw_chan_t (axi_mst_aw_chan_t),
    .mst_aw_chan_t (axi_slv_aw_chan_t),
    .w_chan_t      (axi_mst_w_chan_t ),
    .slv_b_chan_t  (axi_mst_b_chan_t ),
    .mst_b_chan_t  (axi_slv_b_chan_t ),
    .slv_ar_chan_t (axi_mst_ar_chan_t),
    .mst_ar_chan_t (axi_slv_ar_chan_t),
    .slv_r_chan_t  (axi_mst_r_chan_t ),
    .mst_r_chan_t  (axi_slv_r_chan_t ),
    .slv_req_t     (axi_mst_req_t    ),
    .slv_resp_t    (axi_mst_resp_t   ),
    .mst_req_t     (axi_slv_req_t    ),
    .mst_resp_t    (axi_slv_resp_t   ),
    .rule_t        (xbar_rule_t      )
  ) i_axi_dma_xbar (
    .clk_i                 (clk_i                  ),
    .rst_ni                (rst_ni                 ),
    .test_i                (1'b0                   ),
    .slv_ports_req_i       (axi_mst_req            ),
    .slv_ports_resp_o      (axi_mst_rsp            ),
    .mst_ports_req_o       (axi_slv_req            ),
    .mst_ports_resp_i      (axi_slv_rsp            ),
    .addr_map_i            (xbar_rule              ),
    .en_default_mst_port_i (DMAEnableDefaultMstPort),
    .default_mst_port_i    (xbar_default_port      )
  );

  axi_zero_mem #(
    .axi_req_t  (axi_slv_req_t ),
    .axi_resp_t (axi_slv_resp_t),
    .AddrWidth  (AxiAddrWidth  ),
    .DataWidth  (AxiDataWidth  ),
    .IdWidth    (AxiIdWidth    ),
    .NumBanks   (1             ),
    .BufDepth   (1             )
  ) i_axi_zeromem (
    .clk_i      (clk_i                  ),
    .rst_ni     (rst_ni                 ),
    .busy_o     (/* Unused */           ),
    .axi_req_i  (axi_slv_req[ZeroMemory]),
    .axi_resp_o (axi_slv_rsp[ZeroMemory])
  );

  axi_addr_t ext_dma_req_q_addr_nontrunc;

  axi_to_mem_interleaved #(
    .axi_req_t  (axi_slv_req_t         ),
    .axi_resp_t (axi_slv_resp_t        ),
    .AddrWidth  (AxiAddrWidth          ),
    .DataWidth  (AxiDataWidth          ),
    .IdWidth    (AxiIdWidth            ),
    .NumBanks   (1                     ),
    .BufDepth   (MemoryMacroLatency + 1)
  ) i_axi_to_mem_dma (
    .clk_i        (clk_i                                 ),
    .rst_ni       (rst_ni                                ),
    .busy_o       (/* Unused */                          ),
    .axi_req_i    (axi_slv_req[TCDMDMA]                  ),
    .axi_resp_o   (axi_slv_rsp[TCDMDMA]                  ),
    .mem_req_o    (ext_dma_req.q_valid                   ),
    .mem_gnt_i    (ext_dma_rsp.q_ready                   ),
    .mem_addr_o   (ext_dma_req_q_addr_nontrunc           ),
    .mem_wdata_o  (ext_dma_req.q.data                    ),
    .mem_strb_o   (ext_dma_req.q.strb                    ),
    .mem_atop_o   (/* The DMA does not support atomics */),
    .mem_we_o     (ext_dma_req.q.write                   ),
    .mem_rvalid_i (ext_dma_rsp.p_valid                   ),
    .mem_rdata_i  (ext_dma_rsp.p.data                    )
  );

  assign ext_dma_req.q.addr = tcdm_addr_t'(ext_dma_req_q_addr_nontrunc);
  assign ext_dma_req.q.amo  = reqrsp_pkg::AMONone;
  assign ext_dma_req.q.user = '0;

  snitch_tcdm_interconnect #(
    .NumInp                (1                 ),
    .NumOut                (NrSuperBanks      ),
    .tcdm_req_t            (tcdm_dma_req_t    ),
    .tcdm_rsp_t            (tcdm_dma_rsp_t    ),
    .mem_req_t             (mem_dma_req_t     ),
    .mem_rsp_t             (mem_dma_rsp_t     ),
    .user_t                (logic             ),
    .MemAddrWidth          (TCDMMemAddrWidth  ),
    .DataWidth             (AxiDataWidth      ),
    .MemoryResponseLatency (MemoryMacroLatency)
  ) i_dma_interconnect (
    .clk_i     (clk_i      ),
    .rst_ni    (rst_ni     ),
    .req_i     (ext_dma_req),
    .rsp_o     (ext_dma_rsp),
    .mem_req_o (sb_dma_req ),
    .mem_rsp_i (sb_dma_rsp )
  );

  // ----------------
  // Memory Subsystem
  // ----------------
  for (genvar i = 0; i < NrSuperBanks; i++) begin : gen_tcdm_super_bank

    mem_req_t [BanksPerSuperBank-1:0] amo_req;
    mem_rsp_t [BanksPerSuperBank-1:0] amo_rsp;

    mem_wide_narrow_mux #(
      .NarrowDataWidth  (DataWidth     ),
      .WideDataWidth    (AxiDataWidth  ),
      .mem_narrow_req_t (mem_req_t     ),
      .mem_narrow_rsp_t (mem_rsp_t     ),
      .mem_wide_req_t   (mem_dma_req_t ),
      .mem_wide_rsp_t   (mem_dma_rsp_t )
    ) i_tcdm_mux (
      .clk_i           (clk_i                ),
      .rst_ni          (rst_ni               ),
      .in_narrow_req_i (ic_req [i]           ),
      .in_narrow_rsp_o (ic_rsp [i]           ),
      .in_wide_req_i   (sb_dma_req [i]       ),
      .in_wide_rsp_o   (sb_dma_rsp [i]       ),
      .out_req_o       (amo_req              ),
      .out_rsp_i       (amo_rsp              ),
      .sel_wide_i      (sb_dma_req[i].q_valid)
    );

    // generate banks of the superbank
    for (genvar j = 0; j < BanksPerSuperBank; j++) begin : gen_tcdm_bank

      logic mem_cs, mem_wen;
      tcdm_mem_addr_t mem_add;
      axi_strb_t mem_be;
      axi_data_t mem_rdata, mem_wdata;

      tc_sram_impl #(
        .NumWords  (TCDMDepth  ),
        .DataWidth (DataWidth  ),
        .ByteWidth (8          ),
        .NumPorts  (1          ),
        .Latency   (1          ),
        .impl_in_t (sram_cfg_t )
      ) i_data_mem (
        .clk_i   (clk_i           ),
        .rst_ni  (rst_ni          ),
        .impl_i  (sram_cfgs_i.tcdm),
        .impl_o  (/* Unused */    ),
        .req_i   (mem_cs          ),
        .we_i    (mem_wen         ),
        .addr_i  (mem_add         ),
        .wdata_i (mem_wdata       ),
        .be_i    (mem_be          ),
        .rdata_o (mem_rdata       )
      );

      axi_data_t amo_rdata_local;

      // TODO(zarubaf): Share atomic units between mutltiple cuts
      snitch_amo_shim #(
        .AddrMemWidth ( TCDMMemAddrWidth ),
        .DataWidth    ( DataWidth        ),
        .CoreIDWidth  ( CoreIDWidth      )
      ) i_amo_shim (
        .clk_i          (clk_i                     ),
        .rst_ni         (rst_ni                    ),
        .valid_i        (amo_req[j].q_valid        ),
        .ready_o        (amo_rsp[j].q_ready        ),
        .addr_i         (amo_req[j].q.addr         ),
        .write_i        (amo_req[j].q.write        ),
        .wdata_i        (amo_req[j].q.data         ),
        .wstrb_i        (amo_req[j].q.strb         ),
        .core_id_i      (amo_req[j].q.user.core_id ),
        .is_core_i      (amo_req[j].q.user.is_core ),
        .rdata_o        (amo_rdata_local           ),
        .amo_i          (amo_req[j].q.amo          ),
        .mem_req_o      (mem_cs                    ),
        .mem_add_o      (mem_add                   ),
        .mem_wen_o      (mem_wen                   ),
        .mem_wdata_o    (mem_wdata                 ),
        .mem_be_o       (mem_be                    ),
        .mem_rdata_i    (mem_rdata                 ),
        .dma_access_i   (sb_dma_req[i].q_valid     ),
        // TODO(zarubaf): Signal AMO conflict somewhere. Socregs?
        .amo_conflict_o (/* Unused */              )
      );

      // Insert a pipeline register at the output of each SRAM.
      shift_reg #( .dtype (axi_data_t), .Depth (RegisterTCDMCuts)) i_sram_pipe (
        .clk_i, .rst_ni,
        .d_i (amo_rdata_local), .d_o (amo_rsp[j].p.data)
      );
    end
  end

  snitch_tcdm_interconnect #(
    .NumInp                (NumTCDMIn           ),
    .NumOut                (NrBanks             ),
    .tcdm_req_t            (tcdm_req_t          ),
    .tcdm_rsp_t            (tcdm_rsp_t          ),
    .mem_req_t             (mem_req_t           ),
    .mem_rsp_t             (mem_rsp_t           ),
    .MemAddrWidth          (TCDMMemAddrWidth    ),
    .DataWidth             (DataWidth           ),
    .user_t                (tcdm_user_t         ),
    .MemoryResponseLatency (1 + RegisterTCDMCuts)
  ) i_tcdm_interconnect (
    .clk_i     (clk_i                  ),
    .rst_ni    (rst_ni                 ),
    .req_i     ({axi_soc_req, tcdm_req}),
    .rsp_o     ({axi_soc_rsp, tcdm_rsp}),
    .mem_req_o (ic_req                 ),
    .mem_rsp_i (ic_rsp                 )
  );

  hive_req_t [NrCores-1:0] hive_req;
  hive_rsp_t [NrCores-1:0] hive_rsp;

  for (genvar i = 0; i < NrCores; i++) begin : gen_core
    localparam int unsigned TcdmPorts     = get_tcdm_ports(i);
    localparam int unsigned TcdmPortsOffs = get_tcdm_port_offs(i);

    axi_mst_dma_req_t axi_dma_req;
    axi_mst_dma_resp_t axi_dma_res;
    interrupts_t irq;
    dma_events_t dma_core_events;

    sync #(.STAGES (2))
    i_sync_debug (.clk_i, .rst_ni, .serial_i (debug_req_i[i]), .serial_o (irq.debug));
    sync #(.STAGES (2))
    i_sync_meip (.clk_i, .rst_ni, .serial_i (meip_i[i]), .serial_o (irq.meip));
    sync #(.STAGES (2))
    i_sync_mtip (.clk_i, .rst_ni, .serial_i (mtip_i[i]), .serial_o (irq.mtip));
    sync #(.STAGES (2))
    i_sync_msip (.clk_i, .rst_ni, .serial_i (msip_i[i]), .serial_o (irq.msip));
    assign irq.mcip = cl_interrupt[i];

    tcdm_req_t [TcdmPorts-1:0] tcdm_req_wo_user;

    spatz_cc #(
      .BootAddr               (BootAddr                  ),
      .RVE                    (RVE[i]                    ),
      .RVF                    (RVF[i]                    ),
      .RVD                    (RVD[i]                    ),
      .AddrWidth              (PhysicalAddrWidth         ),
      .DataWidth              (NarrowDataWidth           ),
      .DMADataWidth           (WideDataWidth             ),
      .DMAIdWidth             (WideIdWidthIn             ),
      .SnitchPMACfg           (SnitchPMACfg              ),
      .DMAAxiReqFifoDepth     (DMAAxiReqFifoDepth        ),
      .DMAReqFifoDepth        (DMAReqFifoDepth           ),
      .dreq_t                 (reqrsp_req_t              ),
      .drsp_t                 (reqrsp_rsp_t              ),
      .tcdm_req_t             (tcdm_req_t                ),
      .tcdm_rsp_t             (tcdm_rsp_t                ),
      .tcdm_user_t            (tcdm_user_t               ),
      .axi_req_t              (axi_mst_dma_req_t         ),
      .axi_rsp_t              (axi_mst_dma_resp_t        ),
      .hive_req_t             (hive_req_t                ),
      .hive_rsp_t             (hive_rsp_t                ),
      .acc_req_t              (acc_req_t                 ),
      .acc_resp_t             (acc_resp_t                ),
      .dma_events_t           (dma_events_t              ),
      .XDivSqrt               (XDivSqrt[i]               ),
      .XF16                   (XF16[i]                   ),
      .XF16ALT                (XF16ALT[i]                ),
      .XF8                    (XF8[i]                    ),
      .XF8ALT                 (XF8ALT[i]                 ),
      .XFVEC                  (XFVEC[i]                  ),
      .XFDOTP                 (XFDOTP[i]                 ),
      .Xdma                   (Xdma[i]                   ),
      .IsoCrossing            (IsoCrossing               ),
      .Xfrep                  (Xfrep[i]                  ),
      .Xssr                   (Xssr[i]                   ),
      .Xipu                   (1'b0                      ),
      .VMSupport              (VMSupport                 ),
      .NumIntOutstandingLoads (NumIntOutstandingLoads[i] ),
      .NumIntOutstandingMem   (NumIntOutstandingMem[i]   ),
      .NumFPOutstandingLoads  (NumFPOutstandingLoads[i]  ),
      .NumFPOutstandingMem    (NumFPOutstandingMem[i]    ),
      .FPUImplementation      (FPUImplementation[i]      ),
      .NumDTLBEntries         (NumDTLBEntries[i]         ),
      .NumITLBEntries         (NumITLBEntries[i]         ),
      .NumSequencerInstr      (NumSequencerInstr[i]      ),
      .NumSsrs                (NumSsrs[i]                ),
      .SsrMuxRespDepth        (SsrMuxRespDepth[i]        ),
      .SsrCfgs                (SsrCfgs[i][NumSsrs[i]-1:0]),
      .SsrRegs                (SsrRegs[i][NumSsrs[i]-1:0]),
      .RegisterOffloadReq     (RegisterOffloadReq        ),
      .RegisterOffloadRsp     (RegisterOffloadRsp        ),
      .RegisterCoreReq        (RegisterCoreReq           ),
      .RegisterCoreRsp        (RegisterCoreRsp           ),
      .RegisterFPUReq         (RegisterFPUReq            ),
      .RegisterSequencer      (RegisterSequencer         ),
      .RegisterFPUIn          (RegisterFPUIn             ),
      .RegisterFPUOut         (RegisterFPUOut            ),
      .TCDMAddrWidth          (TCDMAddrWidth             )
    ) i_spatz_cc (
      .clk_i            ( clk_i                             ),
      .rst_ni           ( rst_ni                            ),
      .hart_id_i        ( hart_base_id_i + i                ),
      .hive_req_o       ( hive_req[i]                       ),
      .hive_rsp_i       ( hive_rsp[i]                       ),
      .irq_i            ( irq                               ),
      .data_req_o       ( core_req[i]                       ),
      .data_rsp_i       ( core_rsp[i]                       ),
      .tcdm_req_o       ( tcdm_req_wo_user                  ),
      .tcdm_rsp_i       ( tcdm_rsp[TcdmPortsOffs+:TcdmPorts]),
      .axi_dma_req_o    ( axi_dma_req                       ),
      .axi_dma_res_i    ( axi_dma_res                       ),
      .axi_dma_busy_o   (                                   ),
      .axi_dma_perf_o   (                                   ),
      .axi_dma_events_o ( dma_core_events                   ),
      .core_events_o    ( core_events[i]                    ),
      .tcdm_addr_base_i ( tcdm_start_address                )
    );
    for (genvar j = 0; j < TcdmPorts; j++) begin : gen_tcdm_user
      always_comb begin
        tcdm_req[TcdmPortsOffs+j]                = tcdm_req_wo_user[j];
        tcdm_req[TcdmPortsOffs+j].q.user.core_id = i;
        tcdm_req[TcdmPortsOffs+j].q.user.is_core = 1;
      end
    end
    if (Xdma[i]) begin : gen_dma_connection
      assign wide_axi_mst_req[SDMAMst] = axi_dma_req;
      assign axi_dma_res               = wide_axi_mst_rsp[SDMAMst];
      assign dma_events                = dma_core_events;
    end
  end

  for (genvar i = 0; i < NrHives; i++) begin : gen_hive
    localparam int unsigned HiveSize = get_hive_size(i);

    hive_req_t [HiveSize-1:0] hive_req_reshape;
    hive_rsp_t [HiveSize-1:0] hive_rsp_reshape;

    snitch_icache_pkg::icache_events_t [HiveSize-1:0] icache_events_reshape;

    for (genvar j = 0; j < NrCores; j++) begin : gen_hive_matrix
      // Check whether the core actually belongs to the current hive.
      if (Hive[j] == i) begin : gen_hive_connection
        localparam int unsigned HivePosition  = get_core_position(i, j);
        assign hive_req_reshape[HivePosition] = hive_req[j];
        assign hive_rsp[j]                    = hive_rsp_reshape[HivePosition];
        assign icache_events[j]               = icache_events_reshape[HivePosition];
      end
    end

    snitch_hive #(
      .AddrWidth       (AxiAddrWidth      ),
      .NarrowDataWidth (DataWidth         ),
      .WideDataWidth   (AxiDataWidth      ),
      .VMSupport       (1'b0              ),
      .dreq_t          (reqrsp_req_t      ),
      .drsp_t          (reqrsp_rsp_t      ),
      .hive_req_t      (hive_req_t        ),
      .hive_rsp_t      (hive_rsp_t        ),
      .CoreCount       (HiveSize          ),
      .ICacheLineWidth (ICacheLineWidth[i]),
      .ICacheLineCount (ICacheLineCount[i]),
      .ICacheSets      (ICacheSets[i]     ),
      .IsoCrossing     (1'b0              ),
      .sram_cfg_t      (sram_cfg_t        ),
      .sram_cfgs_t     (sram_cfgs_t       ),
      .axi_req_t       (axi_mst_req_t     ),
      .axi_rsp_t       (axi_mst_resp_t    )
    ) i_snitch_hive (
      .clk_i                    (clk_i                 ),
      .rst_ni                   (rst_ni                ),
      .hive_req_i               (hive_req_reshape      ),
      .hive_rsp_o               (hive_rsp_reshape      ),
      .ptw_data_req_o           (ptw_req[i]            ),
      .ptw_data_rsp_i           (ptw_rsp[i]            ),
      .axi_req_o                (axi_mst_req[ICache+i] ),
      .axi_rsp_i                (axi_mst_rsp[ICache+i] ),
      .icache_prefetch_enable_i (icache_prefetch_enable),
      .icache_events_o          (icache_events_reshape ),
      .sram_cfgs_i
    );
  end

  // --------
  // PTW Demux
  // --------
  reqrsp_req_t ptw_to_axi_req;
  reqrsp_rsp_t ptw_to_axi_rsp;

  reqrsp_mux #(
    .NrPorts   (NrHives      ),
    .AddrWidth (AxiAddrWidth ),
    .DataWidth (DataWidth    ),
    .req_t     (reqrsp_req_t ),
    .rsp_t     (reqrsp_rsp_t ),
    .RespDepth (2            )
  ) i_reqrsp_mux_ptw (
    .clk_i     (clk_i            ),
    .rst_ni    (rst_ni           ),
    .slv_req_i (ptw_req          ),
    .slv_rsp_o (ptw_rsp          ),
    .mst_req_o (ptw_to_axi_req   ),
    .mst_rsp_i (ptw_to_axi_rsp   ),
    .idx_o     (/*not connected*/)
  );

  reqrsp_to_axi #(
    .DataWidth    (DataWidth      ),
    .UserWidth    (AxiUserWidth   ),
    .reqrsp_req_t (reqrsp_req_t   ),
    .reqrsp_rsp_t (reqrsp_rsp_t   ),
    .axi_req_t    (axi_mst_req_t  ),
    .axi_rsp_t    (axi_mst_resp_t )
  ) i_reqrsp_to_axi_ptw (
    .clk_i        (clk_i           ),
    .rst_ni       (rst_ni          ),
    .user_i       ('0              ),
    .reqrsp_req_i (ptw_to_axi_req  ),
    .reqrsp_rsp_o (ptw_to_axi_rsp  ),
    .axi_req_o    (axi_mst_req[PTW]),
    .axi_rsp_i    (axi_mst_rsp[PTW])
  );

  // --------
  // Coes SoC
  // --------
  snitch_barrier #(
    .AddrWidth (AxiAddrWidth ),
    .NrPorts   (NrCores      ),
    .dreq_t    (reqrsp_req_t ),
    .drsp_t    (reqrsp_rsp_t )
  ) i_snitch_barrier (
    .clk_i                          (clk_i                       ),
    .rst_ni                         (rst_ni                      ),
    .in_req_i                       (core_req                    ),
    .in_rsp_o                       (core_rsp                    ),
    .out_req_o                      (filtered_core_req           ),
    .out_rsp_i                      (filtered_core_rsp           ),
    .cluster_periph_start_address_i (cluster_periph_start_address)
  );

  reqrsp_req_t core_to_axi_req;
  reqrsp_rsp_t core_to_axi_rsp;
  axi_user_t   cluster_user;
  // Atomic ID, needs to be unique ID of cluster
  // cluster_id + HartIdOffset + 1 (because 0 is for non-atomic masters)
  assign cluster_user = (hart_base_id_i / NrCores) + (hart_base_id_i % NrCores) + 1'b1;

  reqrsp_mux #(
    .NrPorts   (NrCores      ),
    .AddrWidth (AxiAddrWidth ),
    .DataWidth (DataWidth    ),
    .req_t     (reqrsp_req_t ),
    .rsp_t     (reqrsp_rsp_t ),
    .RespDepth (2            )
  ) i_reqrsp_mux_core (
    .clk_i,
    .rst_ni,
    .slv_req_i (filtered_core_req),
    .slv_rsp_o (filtered_core_rsp),
    .mst_req_o (core_to_axi_req  ),
    .mst_rsp_i (core_to_axi_rsp  ),
    .idx_o     (/*unused*/       )
  );

  reqrsp_to_axi #(
    .DataWidth    (DataWidth      ),
    .UserWidth    (AxiUserWidth   ),
    .reqrsp_req_t (reqrsp_req_t   ),
    .reqrsp_rsp_t (reqrsp_rsp_t   ),
    .axi_req_t    (axi_mst_req_t  ),
    .axi_rsp_t    (axi_mst_resp_t )
  ) i_reqrsp_to_axi_core (
    .clk_i,
    .rst_ni,
    .user_i       (cluster_user        ),
    .reqrsp_req_i (core_to_axi_req     ),
    .reqrsp_rsp_o (core_to_axi_rsp     ),
    .axi_req_o    (axi_mst_req[CoreReq]),
    .axi_rsp_i    (axi_mst_rsp[CoreReq])
  );

  logic [ClusterXbarCfg.NoSlvPorts-1:0][$clog2(ClusterXbarCfg.NoMstPorts)-1:0]
  cluster_xbar_default_port;
  xbar_rule_t [NrRules-1:0] cluster_xbar_rules;

  assign cluster_xbar_rules = '{
    '{
      idx       : TCDM,
      start_addr: tcdm_start_address,
      end_addr  : tcdm_end_address
    },
    '{
      idx       : ClusterPeripherals,
      start_addr: cluster_periph_start_address,
      end_addr  : cluster_periph_end_address
    }
  };

  localparam bit [ClusterXbarCfg.NoSlvPorts-1:0] ClusterEnableDefaultMstPort = '1;
  axi_xbar #(
    .Cfg           (ClusterXbarCfg   ),
    .slv_aw_chan_t (axi_mst_aw_chan_t),
    .mst_aw_chan_t (axi_slv_aw_chan_t),
    .w_chan_t      (axi_mst_w_chan_t ),
    .slv_b_chan_t  (axi_mst_b_chan_t ),
    .mst_b_chan_t  (axi_slv_b_chan_t ),
    .slv_ar_chan_t (axi_mst_ar_chan_t),
    .mst_ar_chan_t (axi_slv_ar_chan_t),
    .slv_r_chan_t  (axi_mst_r_chan_t ),
    .mst_r_chan_t  (axi_slv_r_chan_t ),
    .slv_req_t     (axi_mst_req_t    ),
    .slv_resp_t    (axi_mst_resp_t   ),
    .mst_req_t     (axi_slv_req_t    ),
    .mst_resp_t    (axi_slv_resp_t   ),
    .rule_t        (xbar_rule_t      )
  ) i_cluster_xbar (
    .clk_i,
    .rst_ni,
    .test_i                (1'b0                       ),
    .slv_ports_req_i       (axi_mst_req                ),
    .slv_ports_resp_o      (axi_mst_rsp                ),
    .mst_ports_req_o       (axi_slv_req                ),
    .mst_ports_resp_i      (axi_slv_rsp                ),
    .addr_map_i            (cluster_xbar_rules         ),
    .en_default_mst_port_i (ClusterEnableDefaultMstPort),
    .default_mst_port_i    (cluster_xbar_default_port  )
  );
  assign cluster_xbar_default_port = '{default: SoC};

  // Optionally decouple the external narrow AXI slave port.
  axi_cut #(
    .Bypass     (!RegisterExtNarrow),
    .aw_chan_t  (axi_mst_aw_chan_t ),
    .w_chan_t   (axi_mst_w_chan_t  ),
    .b_chan_t   (axi_mst_b_chan_t  ),
    .ar_chan_t  (axi_mst_ar_chan_t ),
    .r_chan_t   (axi_mst_r_chan_t  ),
    .axi_req_t  (axi_mst_req_t     ),
    .axi_resp_t (axi_mst_resp_t    )
  ) i_cut_ext_narrow_slv (
    .clk_i,
    .rst_ni,
    .slv_req_i  (narrow_in_req_i    ),
    .slv_resp_o (narrow_in_resp_o   ),
    .mst_req_o  (axi_mst_req[AXISoC]),
    .mst_resp_i (axi_mst_rsp[AXISoC])
  );

  // ---------
  // Slaves
  // ---------
  // 1. TCDM
  // Add an adapter that allows access from AXI to the TCDM.
  axi_to_tcdm #(
    .axi_req_t  (axi_slv_req_t         ),
    .axi_rsp_t  (axi_slv_resp_t        ),
    .tcdm_req_t (tcdm_req_t            ),
    .tcdm_rsp_t (tcdm_rsp_t            ),
    .AddrWidth  (AxiAddrWidth          ),
    .DataWidth  (AxiDataWidth          ),
    .IdWidth    (AxiIdWidth            ),
    .BufDepth   (MemoryMacroLatency + 1)
  ) i_axi_to_tcdm (
    .clk_i      (clk_i            ),
    .rst_ni     (rst_ni           ),
    .axi_req_i  (axi_slv_req[TCDM]),
    .axi_rsp_o  (axi_slv_rsp[TCDM]),
    .tcdm_req_o (axi_soc_req      ),
    .tcdm_rsp_i (axi_soc_rsp      )
  );

  // 2. Peripherals
  axi_to_reg #(
    .ADDR_WIDTH         (AxiAddrWidth  ),
    .DATA_WIDTH         (DataWidth     ),
    .AXI_MAX_WRITE_TXNS (1             ),
    .AXI_MAX_READ_TXNS  (1             ),
    .DECOUPLE_W         (0             ),
    .ID_WIDTH           (AxiIdWidth    ),
    .USER_WIDTH         (AxiUserWidth  ),
    .axi_req_t          (axi_slv_req_t ),
    .axi_rsp_t          (axi_slv_resp_t),
    .reg_req_t          (reg_req_t     ),
    .reg_rsp_t          (reg_rsp_t     )
  ) i_axi_to_reg (
    .clk_i,
    .rst_ni,
    .testmode_i (1'b0                           ),
    .axi_req_i  (axi_slv_req[ClusterPeripherals]),
    .axi_rsp_o  (axi_slv_rsp[ClusterPeripherals]),
    .reg_req_o  (reg_req                        ),
    .reg_rsp_i  (reg_rsp                        )
  );

  snitch_cluster_peripheral #(
    .AddrWidth     (AxiAddrWidth  ),
    .reg_req_t     (reg_req_t     ),
    .reg_rsp_t     (reg_rsp_t     ),
    .tcdm_events_t (tcdm_events_t ),
    .dma_events_t  (dma_events_t  ),
    .NrCores       (NrCores       )
  ) i_snitch_cluster_peripheral (
    .clk_i                    (clk_i                 ),
    .rst_ni                   (rst_ni                ),
    .reg_req_i                (reg_req               ),
    .reg_rsp_o                (reg_rsp               ),
    /// The TCDM always starts at the cluster base.
    .tcdm_start_address_i     (tcdm_start_address    ),
    .tcdm_end_address_i       (tcdm_end_address      ),
    .icache_prefetch_enable_o (icache_prefetch_enable),
    .cl_clint_o               (cl_interrupt          ),
    .cluster_hart_base_id_i   (hart_base_id_i        ),
    .core_events_i            (core_events           ),
    .tcdm_events_i            (tcdm_events           ),
    .dma_events_i             (dma_events            ),
    .icache_events_i          (icache_events         )
  );

  // Optionally decouple the external narrow AXI master ports.
  axi_cut #(
    .Bypass     ( !RegisterExt      ),
    .aw_chan_t  ( axi_slv_aw_chan_t ),
    .w_chan_t   ( axi_slv_w_chan_t  ),
    .b_chan_t   ( axi_slv_b_chan_t  ),
    .ar_chan_t  ( axi_slv_ar_chan_t ),
    .r_chan_t   ( axi_slv_r_chan_t  ),
    .axi_req_t  ( axi_slv_req_t     ),
    .axi_resp_t ( axi_slv_resp_t    )
  ) i_cut_ext_narrow_mst (
    .clk_i      ( clk_i             ),
    .rst_ni     ( rst_ni            ),
    .slv_req_i  ( axi_slv_req[SoC]  ),
    .slv_resp_o ( axi_slv_rsp[SoC]  ),
    .mst_req_o  ( narrow_out_req_o  ),
    .mst_resp_i ( narrow_out_resp_i )
  );

  // --------------------
  // TCDM event counters
  // --------------------
  logic [NrTCDMPortsCores-1:0] flat_acc, flat_con;
  for (genvar i = 0; i < NrTCDMPortsCores; i++) begin : gen_event_counter
    `FFARN(flat_acc[i], tcdm_req[i].q_valid, '0, clk_i, rst_ni)
    `FFARN(flat_con[i], tcdm_req[i].q_valid & ~tcdm_rsp[i].q_ready, '0, clk_i, rst_ni)
  end

  popcount #(
    .INPUT_WIDTH ( NrTCDMPortsCores )
  ) i_popcount_req (
    .data_i     ( flat_acc                 ),
    .popcount_o ( tcdm_events.inc_accessed )
  );

  popcount #(
    .INPUT_WIDTH ( NrTCDMPortsCores )
  ) i_popcount_con (
    .data_i     ( flat_con                  ),
    .popcount_o ( tcdm_events.inc_congested )
  );

  // -------------
  // Sanity Checks
  // -------------
  // Sanity check the parameters. Not every configuration makes sense.
  `ASSERT_INIT(CheckSuperBankSanity, NrBanks >= BanksPerSuperBank);
  `ASSERT_INIT(CheckSuperBankFactor, (NrBanks % BanksPerSuperBank) == 0);
  // Check that the cluster base address aligns to the TCDMSize.
  `ASSERT(ClusterBaseAddrAlign, ((TCDMSize - 1) & cluster_base_addr_i) == 0)
  // Make sure we only have one DMA in the system.
  `ASSERT_INIT(NumberDMA, $onehot0(Xdma))

endmodule
