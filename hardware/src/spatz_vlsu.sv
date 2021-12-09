// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Domenic Wüthrich, ETH Zurich

module spatz_vlsu
  import spatz_pkg::*;
  import rvv_pkg::*;
  import cf_math_pkg::idx_width;
#(
  parameter NR_MEM_PORTS         = 1,
  parameter NR_OUTSTANDING_LOADS = 8,
  parameter type x_mem_req_t     = logic,
  parameter type x_mem_resp_t    = logic,
  parameter type x_mem_result_t  = logic,
  // Dependant parameters. DO NOT CHANGE!
  localparam int unsigned IdWidth = idx_width(NR_OUTSTANDING_LOADS)
) (
  input  logic clk_i,
  input  logic rst_ni,
  // Spatz req
  input  spatz_req_t spatz_req_i,
  input  logic       spatz_req_valid_i,
  output logic       spatz_req_ready_o,
  // VFU rsp
  output logic      vlsu_rsp_valid_o,
  output vlsu_rsp_t vlsu_rsp_o,
  // VRF
  output vreg_addr_t vrf_waddr_o,
  output vreg_data_t vrf_wdata_o,
  output logic       vrf_we_o,
  output vreg_be_t   vrf_wbe_o,
  input  logic       vrf_wvalid_i,
  output vreg_addr_t vrf_raddr_o,
  output logic       vrf_re_o,
  input  vreg_data_t vrf_rdata_i,
  input  logic       vrf_rvalid_i,
  // X-Interface Memory Request
  output logic          [NR_MEM_PORTS-1:0] x_mem_valid_o,
  input  logic          [NR_MEM_PORTS-1:0] x_mem_ready_i,
  output x_mem_req_t    [NR_MEM_PORTS-1:0] x_mem_req_o,
  input  x_mem_resp_t   [NR_MEM_PORTS-1:0] x_mem_resp_i,
  //X-Interface Memory Result
  input  logic          [NR_MEM_PORTS-1:0] x_mem_result_valid_i,
  input  x_mem_result_t [NR_MEM_PORTS-1:0] x_mem_result_i
);

  // Include FF
  `include "common_cells/registers.svh"

  //////////////
  // Typedefs //
  //////////////

  typedef logic [IdWidth-1:0]        id_t;
  typedef logic [$clog2(VELE*8)-1:0] vreg_elem_t;

  /////////////
  // Signals //
  /////////////

  // Spatz request
  spatz_req_t spatz_req_d, spatz_req_q;
  `FF(spatz_req_q, spatz_req_d, '0)

  // Is vfu and the ipu operands ready
  logic vlsu_is_ready;

  // Has a new vfu execution request arrived
  logic new_vlsu_request;
  assign new_vlsu_request = spatz_req_valid_i && vlsu_is_ready && (spatz_req_i.ex_unit == LSU);

  // Is instruction a load
  logic is_load;
  assign is_load = (spatz_req_q.op == VLE) || (spatz_req_q.op == VLSE) || (spatz_req_q.op == VLXE);

  logic  is_vl_zero;
  assign is_vl_zero = spatz_req_q.vl == 'd0;

  logic is_addr_unaligned;
  assign is_addr_unaligned = spatz_req_q.rs1[1:0] != 2'b00;

  assign spatz_req_ready_o = vlsu_is_ready;

  assign vlsu_rsp_valid_o = '0;
  assign vlsu_rsp_o = '0;

  elen_t [NR_MEM_PORTS-1:0] buffer_wdata;
  id_t   [NR_MEM_PORTS-1:0] buffer_wid;
  logic  [NR_MEM_PORTS-1:0] buffer_push;
  logic  [NR_MEM_PORTS-1:0] buffer_rvalid;
  elen_t [NR_MEM_PORTS-1:0] buffer_rdata;
  logic  [NR_MEM_PORTS-1:0] buffer_pop;
  id_t   [NR_MEM_PORTS-1:0] buffer_rid;
  logic  [NR_MEM_PORTS-1:0] buffer_req_id;
  id_t   [NR_MEM_PORTS-1:0] buffer_id;
  logic  [NR_MEM_PORTS-1:0] buffer_full;
  logic  [NR_MEM_PORTS-1:0] buffer_empty;

  logic  [NR_MEM_PORTS-1:0] mem_counter_clear;
  logic  [NR_MEM_PORTS-1:0] mem_counter_en;
  logic  [NR_MEM_PORTS-1:0] mem_counter_load;
  vlen_t [NR_MEM_PORTS-1:0] mem_counter_delta;
  vlen_t [NR_MEM_PORTS-1:0] mem_counter_load_value;
  vlen_t [NR_MEM_PORTS-1:0] mem_counter_value;
  logic  [NR_MEM_PORTS-1:0] mem_counter_overflow;

  logic  vreg_counter_clear;
  logic  vreg_counter_en;
  logic  vreg_counter_load;
  vlen_t vreg_counter_delta;
  vlen_t vreg_counter_load_value;
  vlen_t vreg_counter_value;
  logic  vreg_counter_overflow;

  vreg_elem_t vreg_elem_id;

  logic [NR_MEM_PORTS-1:0] mem_operation_valid;
  logic                    vreg_operation_valid;

  ///////////////////
  // State Handler //
  ///////////////////

  always_comb begin
    spatz_req_d = spatz_req_q;

    if (new_vlsu_request) begin
      spatz_req_d = spatz_req_i;
    end else if (!is_vl_zero) begin
      if (vlsu_is_ready) begin
        spatz_req_d = new_vlsu_request ? spatz_req_i : '0;
      end
    end
  end

  assign vlsu_is_ready = ~vreg_operation_valid & ~(|mem_operation_valid);

  ////////////////////////
  // Address Generation //
  ////////////////////////

  elen_t      [NR_MEM_PORTS-1:0] mem_req_addr;
  vreg_addr_t [NR_MEM_PORTS-1:0] vreg_addr;
  logic       [NR_MEM_PORTS-1:0][$clog2(NR_OUTSTANDING_LOADS)-1:0] buffer_id_diff;

  always_comb begin : gen_mem_req_addr
    for (int unsigned i = 0; i < NR_MEM_PORTS; i++) begin : gen_elem_access
      mem_req_addr[i] = '0;
      buffer_id_diff[i] = buffer_id[i] - buffer_rid[i];

      unique case (spatz_req_q.op)
        VLE: begin
          automatic int unsigned offset = ((vreg_elem_id << $clog2(N_IPU)) + ($unsigned(buffer_id_diff[i]) << $clog2(NR_MEM_PORTS)) + i) << 2;
          mem_req_addr[i] = spatz_req_q.rs1 + offset;
        end
        VSE: begin
          automatic int unsigned offset = ((vreg_elem_id << $clog2(N_IPU)) - ($unsigned(buffer_id_diff[i]) << $clog2(NR_MEM_PORTS)) + i) << 2;
          mem_req_addr[i] = spatz_req_q.rs1 + offset;
        end
        VLSE: begin
          automatic int unsigned offset = ((vreg_elem_id << $clog2(N_IPU)) + ($unsigned(buffer_id_diff[i]) << $clog2(NR_MEM_PORTS)) + i) << 2;
          mem_req_addr[i] = spatz_req_q.rs1 + offset * $signed(spatz_req_q.rs2);
        end
        VSSE: begin
          automatic int unsigned offset = ((vreg_elem_id << $clog2(N_IPU)) - ($unsigned(buffer_id_diff[i]) << $clog2(NR_MEM_PORTS)) + i) << 2;
          mem_req_addr[i] = spatz_req_q.rs1 + offset * $signed(spatz_req_q.rs2);
        end
        default: begin
          mem_req_addr[i] = '0;
        end
      endcase
    end
  end

  always_comb begin : gen_vreg_addr
    vreg_addr = '0;

    unique case (spatz_req_q.op)
      VLE,
      VSE,
      VLSE,
      VSSE: begin
        vreg_addr = {spatz_req_q.vd, {$clog2(VELE){1'b0}}} + $unsigned(vreg_elem_id);
      end
      default: begin
        vreg_addr = '0;
      end
    endcase
  end

  //////////////
  // Counters //
  //////////////

  delta_counter #(
    .WIDTH($bits(vlen_t))
  ) i_delta_counter_vreg (
    .clk_i     (clk_i),
    .rst_ni    (rst_ni),
    .clear_i   (vreg_counter_clear),
    .en_i      (vreg_counter_en),
    .load_i    (vreg_counter_load),
    .down_i    (1'b0), // We always count up
    .delta_i   (vreg_counter_delta),
    .d_i       (vreg_counter_load_value),
    .q_o       (vreg_counter_value),
    .overflow_o(vreg_counter_overflow)
  );

  always_comb begin
    vreg_counter_load = 1'b0;
    vreg_counter_load_value = '0;

    vreg_counter_clear = new_vlsu_request;
    vreg_counter_delta = ew_to_bytes(spatz_req_q.vtype.vsew) << $clog2(NR_MEM_PORTS);
    vreg_counter_en = (is_load & vrf_wvalid_i & vrf_we_o) | (~is_load & vrf_rvalid_i & vrf_re_o);

    vreg_operation_valid = (vreg_counter_value <= spatz_req_q.vl) & ~is_vl_zero;

    vreg_elem_id = vreg_counter_value >> $clog2(ELENB*N_IPU);
  end

  for (genvar i = 0; i < NR_MEM_PORTS; i++) begin
    delta_counter #(
      .WIDTH($bits(vlen_t))
    ) i_delta_counter_mem (
      .clk_i     (clk_i),
      .rst_ni    (rst_ni),
      .clear_i   (mem_counter_clear[i]),
      .en_i      (mem_counter_en[i]),
      .load_i    (mem_counter_load[i]),
      .down_i    (1'b0), // We always count up
      .delta_i   (mem_counter_delta[i]),
      .d_i       (mem_counter_load_value[i]),
      .q_o       (mem_counter_value[i]),
      .overflow_o(mem_counter_overflow[i])
    );
  end

  always_comb begin
    for (int unsigned i = 0; i < NR_MEM_PORTS; i++) begin
      mem_counter_load[i] = 1'b0;
      mem_counter_load_value[i] = '0;

      mem_counter_clear[i] = new_vlsu_request;
      mem_counter_delta[i] = ew_to_bytes(spatz_req_q.vtype.vsew);
      mem_counter_en[i] = x_mem_ready_i[i];

      mem_operation_valid[i] = (mem_counter_value[i] < ((spatz_req_q.vl >> $clog2(NR_MEM_PORTS)) + (spatz_req_q.vl[$clog2(NR_MEM_PORTS)-1:0] & (i + 'd1)))) & ~is_vl_zero;
    end
  end

  ////////////////////
  // Reorder Buffer //
  ////////////////////

  for (genvar i = 0; i < NR_MEM_PORTS; i++) begin
    reorder_buffer #(
      .DataWidth  (ELEN),
      .NumWords   (NR_OUTSTANDING_LOADS),
      .FallThrough(1'b0)
    ) i_reorder_buffer (
      .clk_i    (clk_i),
      .rst_ni   (rst_ni),
      .data_i   (buffer_wdata[i]),
      .id_i     (buffer_wid[i]),
      .push_i   (buffer_push[i]),
      .data_o   (buffer_rdata[i]),
      .valid_o  (buffer_rvalid[i]),
      .id_read_o(buffer_rid[i]),
      .pop_i    (buffer_pop[i]),
      .id_req_i (buffer_req_id[i]),
      .id_o     (buffer_id[i]),
      .full_o   (buffer_full[i]),
      .empty_o  (buffer_empty[i])
    );
  end

  ///////////////////////////////
  // Memory/Vregfile Interface //
  ///////////////////////////////

  id_t  [NR_MEM_PORTS-1:0]             mem_req_id;
  logic [NR_MEM_PORTS-1:0]             mem_req_svalid;
  logic [NR_MEM_PORTS-1:0][ELEN/8-1:0] mem_req_strb;
  logic [NR_MEM_PORTS-1:0]             mem_req_lvalid;
  logic [NR_MEM_PORTS-1:0]             mem_req_last;

  always_comb begin
    vrf_raddr_o = '0;
    vrf_re_o    = 1'b0;
    vrf_waddr_o = '0;
    vrf_wdata_o = '0;
    vrf_we_o    = 1'b0;
    vrf_wbe_o   = '0;

    buffer_wdata  = '0;
    buffer_wid    = '0;
    buffer_push   = '0;
    buffer_pop    = '0;
    buffer_req_id = '0;

    mem_req_id     = '0;
    mem_req_strb   = '0;
    mem_req_svalid = '0;
    mem_req_lvalid = '0;
    mem_req_last   = '1;

    if (is_load) begin
      // If we have a valid element in the buffer,
      // store it back to the register file
      if (&buffer_rvalid & vreg_operation_valid) begin
        vrf_waddr_o = vreg_addr;
        vrf_wdata_o = buffer_rdata;
        vrf_we_o = 1'b1;
        vrf_wbe_o = '1;

        // Pop stored element and free space in buffer
        buffer_pop = {NR_MEM_PORTS{vrf_wvalid_i}};
      end

      for (int unsigned i = 0; i < NR_MEM_PORTS; i++) begin
        // Request a new id and and execute memory request
        if (~buffer_full[i] && mem_operation_valid[i]) begin
          buffer_req_id[i] = x_mem_ready_i[i];
          mem_req_lvalid[i] = 1'b1;
          mem_req_id[i] = buffer_id[i];
        end

        // Write the load result to the buffer
        buffer_wdata[i] = x_mem_result_i[i].rdata;
        buffer_wid[i] = x_mem_result_i[i].id;
        buffer_push[i] = x_mem_result_valid_i[i];
      end
    end else begin
      // Read new element from the register file and store
      // it to the buffer
      if (~(|buffer_full) & vreg_operation_valid) begin
        vrf_raddr_o = vreg_addr;
        vrf_re_o = 1'b1;

        // Push element to buffer if read from vregfile
        buffer_req_id = '1;
        buffer_wid = buffer_id;
        buffer_wdata = vrf_rdata_i;
        buffer_push = {NR_MEM_PORTS{vrf_rvalid_i}};
      end

      for (int unsigned i = 0; i < NR_MEM_PORTS; i++) begin
        // Read element from buffer and execute memory request
        mem_req_svalid[i] = buffer_rvalid[i];
        mem_req_strb[i] = '1;
        mem_req_id[i] = buffer_rid[i];
        buffer_pop[i] = x_mem_ready_i[i];
      end
    end
  end

  // Create memory requests
  for (genvar i = 0; i < NR_MEM_PORTS; i++) begin : gen_mem_req
    assign x_mem_req_o[i].id    = mem_req_id[i];
    assign x_mem_req_o[i].addr  = mem_req_addr[i];
    assign x_mem_req_o[i].mode  = '0; // Request always uses user privilege level
    assign x_mem_req_o[i].size  = spatz_req_q.vtype.vsew[1:0];
    assign x_mem_req_o[i].we    = ~is_load;
    assign x_mem_req_o[i].strb  = mem_req_strb[i];
    assign x_mem_req_o[i].wdata = buffer_rdata[i];
    assign x_mem_req_o[i].last  = mem_req_last;
    assign x_mem_req_o[i].spec  = 1'b0; // Request is never speculative
    assign x_mem_valid_o[i]     = mem_req_svalid[i] | mem_req_lvalid[i];
  end

  ////////////////
  // Assertions //
  ////////////////

  if (NR_MEM_PORTS > $bits(vrf_wdata_o)/$bits(x_mem_result_i[0].rdata))
    $error("[spatz_vlsu] There are too many spatz memory ports. Consider reducing NR_MEM_PORTS parameter.");

endmodule : spatz_vlsu
