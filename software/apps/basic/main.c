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

// Author: Domenic Wüthrich, ETH Zurich

// clang-format off

#include <stdint.h>

int main() {

  uint32_t vlen = 128;
  uint32_t actual_vlen = 0;
  uint32_t a = 0x10;
  uint32_t csr;
  uint32_t val = 64;

  asm volatile("vsetvli %0, %1, e8, m4, ta, ma" : "=r"(actual_vlen) : "r"(vlen));
  asm volatile("vrsub.vx v8, v0, %[a]" :: [a]"r"(a));
  asm volatile("vadd.vi v4, v8, 5");
  // Clear registers and execute macc
  asm volatile("vadd.vi v0, v0, 2");
  asm volatile("vadd.vx v12, v12, %[val]" :: [val]"r"(val));
  asm volatile("vadd.vi v28, v28, 5");
  asm volatile("vmacc.vv v28, v0, v12");
  asm volatile("vadd.vi v22, v28, 0");
  asm volatile("csrrs %[csr], vl, x0" : [csr]"=r"(csr));

  return actual_vlen-csr;
}

// clang-format on
