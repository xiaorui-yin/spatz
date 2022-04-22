// Copyright 2020 ETH Zurich and University of Bologna.
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

// Author: Matteo Perotti

#ifndef CONV3D_H
#define CONV3D_H

#include <stdint.h>
#include <stdio.h>

void conv3d_CHx3x3(int32_t *o, int32_t *i, int32_t *f);
void conv3d_CHx3x3_block(int32_t *o, int32_t *i, uint32_t num_rows, int32_t *f, uint32_t n_);

void conv3d_CHx7x7(int32_t *o, int32_t *i, int32_t *f);
void conv3d_CHx7x7_block(int32_t *o, int32_t *i, uint32_t num_rows, int32_t *f, uint32_t n_);

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif