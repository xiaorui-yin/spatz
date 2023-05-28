// Copyright 2021 ETH Zurich and University of Bologna.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Matteo Perotti, Marco Bertuletti, ETH Zurich

#include <benchmark.h>
#include <snrt.h>
#include <stdio.h>

#include DATAHEADER
#include "kernel/fft.c"

float *samples;
float *buffer;
float *output;
float *twiddle;

uint32_t *store_idx;
uint32_t *bitrev;

static inline int fp_check(const float a, const float b) {
  const float threshold = 0.00001;

  // Absolute value
  float comp = a - b;
  if (comp < 0)
    comp = -comp;

  return comp > threshold;
}

int main() {
  const unsigned int num_cores = snrt_cluster_core_num();
  const unsigned int cid = snrt_cluster_core_idx();

  unsigned int timer;
  unsigned int log2_nfft;

  // Reset timer
  timer = (unsigned int)-1;

  // log2(nfft).
  log2_nfft = 31 - __builtin_clz(NFFT >> 1);

  // Allocate the matrices
  if (cid == 0) {
    samples = (float *)snrt_l1alloc(2 * NFFT * sizeof(float));
    buffer = (float *)snrt_l1alloc(2 * NFFT * sizeof(float));
    output = (float *)snrt_l1alloc(2 * NFFT * sizeof(float));
    twiddle = (float *)snrt_l1alloc(2 * (NTWI + NFFT) * sizeof(float));
    store_idx =
        (uint32_t *)snrt_l1alloc(log2_nfft * (NFFT / 2) * sizeof(uint32_t));
    bitrev = (uint32_t *)snrt_l1alloc(NFFT * sizeof(uint32_t));
  }

  // Initialize the matrices
  if (cid == 0) {
    snrt_dma_start_1d(samples, samples_dram, 2 * NFFT * sizeof(uint32_t));
    snrt_dma_start_1d(buffer, buffer_dram, 2 * NFFT * sizeof(uint32_t));
    snrt_dma_start_1d(twiddle, twiddle_dram,
                      2 * (NTWI + NFFT) * sizeof(uint32_t));
    snrt_dma_start_1d(store_idx, store_idx_dram,
                      log2_nfft * (NFFT / 2) * sizeof(uint32_t));
    snrt_dma_start_1d(bitrev, bitrev_dram, NFFT * sizeof(uint32_t));
    snrt_dma_wait_all();
  }

  // Wait for all cores to finish
  snrt_cluster_hw_barrier();

  // Calculate pointers for the second butterfly onwards
  float *s_ = samples + cid * NFFT;
  float *buf_ = buffer + cid * NFFT;
  float *out_ = output + cid * NFFT;
  float *twi_ = twiddle + (NFFT >> 1);
  uint32_t nfft_ = NFFT >> 1;

  // Wait for all cores to finish
  snrt_cluster_hw_barrier();

  // Start timer
  timer = benchmark_get_cycle();

  // Start dump
  if (cid == 0)
    start_kernel();

  // First stage
  fft_2c(samples, twiddle, NFFT, cid);

  // Wait for all cores to finish the first stage
  snrt_cluster_hw_barrier();

  // Fall back into the single-core case
  fft_sc(s_, buf_, twi_, out_, store_idx, bitrev, nfft_);

  // Wait for all cores to finish fft
  snrt_cluster_hw_barrier();

  // End dump
  if (cid == 0)
    stop_kernel();

  // End timer and check if new best runtime
  if (cid == 0)
    timer = benchmark_get_cycle() - timer;

  // Display runtime
  if (cid == 0) {
    printf("\n----- fft on %d samples -----\n", NFFT);
    printf("The execution took %u cycles.\n", timer);

    // Verify both real and imaginary parts
    for (unsigned int i = 0; i < 2 * NFFT; i++) {
      if (fp_check(output[i], gold_out_dram[i])) {
        printf("[ERROR] Index %d -> Result = %f, Expected = %f\n", i, output[i],
               gold_out_dram[i]);
      }
    }
  }

  // Wait for core 0 to finish displaying results
  snrt_cluster_hw_barrier();

  return 0;
}
