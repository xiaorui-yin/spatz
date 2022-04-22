#ifndef CONV2D_H
#define CONV2D_H

#include <stdint.h>

inline void conv2d_3x3(int32_t *o, int32_t *i, int32_t *f, int32_t num_rows, int32_t num_columns) __attribute__((always_inline));

void conv2d_5x5(int32_t *o, int32_t *i, int32_t *f, int32_t R, int32_t C,
                int32_t F);
void conv2d_vec_4xC_slice_init_5x5(int32_t *o, int32_t C);
void conv2d_vec_4xC_slice_preload_5x5(int32_t *i, int32_t C, int32_t F);
void conv2d_vec_4xC_slice_move_5x5(int32_t C, int32_t F);
void conv2d_vec_4xC_5x5(int32_t *o, int32_t *i, int32_t *f, int32_t C,
                        int32_t F);

inline void conv2d_7x7(int32_t *o, int32_t *i, int32_t *f, int32_t num_rows, int32_t num_columns) __attribute__((always_inline));

#endif