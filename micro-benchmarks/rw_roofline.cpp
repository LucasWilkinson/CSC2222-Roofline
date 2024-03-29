/******************************************************************************
 *
 * File: rw_roofline.cpp
 * Description: Contains roofline kernel.
 *
 * Author: Alif Ahmed
 * Date: Sep 16, 2019
 *
 * Source: https://github.com/alifahmed/hopscotch/blob/master/cpu/kernels/rw_roofline.cpp
 * Adapted by: Lucas Wilkinson
 * Date: Oct 13, 2021
 *****************************************************************************/

#include "rw_roofline.h"

#include <cfloat>
#include <immintrin.h>


// Select intrinsic based on supported ISA extensions and data type
#if defined(__AVX512F__)
#define VEC_WIDTH               512
    #if ELEM_SIZE == 4
        #define VEC_DATA_T          __m512
        #define VEC_FUNC(func,...)  _mm512_##func##_ps(__VA_ARGS__)
    #elif ELEM_SIZE == 8
        #define VEC_DATA_T          __m512d
        #define VEC_FUNC(func,...)  _mm512_##func##_pd(__VA_ARGS__)
    #endif
#elif defined(__AVX2__) || defined(__AVX__)
#define VEC_WIDTH               256
    #if ELEM_SIZE == 4
        #define VEC_DATA_T          __m256
        #define VEC_FUNC(func,...)  _mm256_##func##_ps(__VA_ARGS__)
    #elif ELEM_SIZE == 8
        #define VEC_DATA_T          __m256d
        #define VEC_FUNC(func,...)  _mm256_##func##_pd(__VA_ARGS__)
    #endif
#else
#define VEC_WIDTH               128
#if ELEM_SIZE == 4
#define VEC_DATA_T          __m128
        #define VEC_FUNC(func,...)  _mm128_##func##_ps(__VA_ARGS__)
#elif ELEM_SIZE == 8
#define VEC_DATA_T          __m128d
        #define VEC_FUNC(func,...)  _mm128_##func##_pd(__VA_ARGS__)
#endif
#endif

#define VEC_ELEMS                   (VEC_WIDTH/8/sizeof(data_t))


/**
 * Roofline kernels
 */

#define KERNEL_SIGNATURE(optimization) \
  template<size_t FLOPS_PER_ELEM> void rw_roofline_##optimization (data_t* __restrict__ x)

KERNEL_SIGNATURE(vectorized_balanced) {
  // Set constants for multiply and add
  const VEC_DATA_T rv = VEC_FUNC(set1, 1.0 + 1e-6);
  const VEC_DATA_T sv = VEC_FUNC(set1, 1e-6);

  // Run kernel. Manually unrolled 8 times to have enough independent
  // instrustions in the reservation station to keep all the FMA units
  // busy. Unrolling more may result in register spilling.
#pragma omp parallel for
  for (uint64_t i = 0; i < WSS_ELEMS; i += VEC_ELEMS * 8) {
    // Load 8 array elements into registers
    VEC_DATA_T xv1 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 0)]);
    VEC_DATA_T xv2 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 1)]);
    VEC_DATA_T xv3 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 2)]);
    VEC_DATA_T xv4 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 3)]);
    VEC_DATA_T xv5 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 4)]);
    VEC_DATA_T xv6 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 5)]);
    VEC_DATA_T xv7 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 6)]);
    VEC_DATA_T xv8 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 7)]);

    // Do FMA operations required number of times
    for (uint64_t j = 0; j < FLOPS_PER_ELEM / 2; ++j) {
      xv1 = VEC_FUNC(fmadd, xv1, rv, sv);
      xv2 = VEC_FUNC(fmadd, xv2, rv, sv);
      xv3 = VEC_FUNC(fmadd, xv3, rv, sv);
      xv4 = VEC_FUNC(fmadd, xv4, rv, sv);
      xv5 = VEC_FUNC(fmadd, xv5, rv, sv);
      xv6 = VEC_FUNC(fmadd, xv6, rv, sv);
      xv7 = VEC_FUNC(fmadd, xv7, rv, sv);
      xv8 = VEC_FUNC(fmadd, xv8, rv, sv);
    }

    // Do an additional operation if FLOPS_PER_ELEM is odd
    if ((FLOPS_PER_ELEM & 1) == 1) {
      xv1 = VEC_FUNC(add, xv1, sv);
      xv2 = VEC_FUNC(add, xv2, sv);
      xv3 = VEC_FUNC(add, xv3, sv);
      xv4 = VEC_FUNC(add, xv4, sv);
      xv5 = VEC_FUNC(add, xv5, sv);
      xv6 = VEC_FUNC(add, xv6, sv);
      xv7 = VEC_FUNC(add, xv7, sv);
      xv8 = VEC_FUNC(add, xv8, sv);
    }

    // Store the result
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 0)], xv1);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 1)], xv2);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 2)], xv3);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 3)], xv4);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 4)], xv5);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 5)], xv6);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 6)], xv7);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 7)], xv8);
  }
}

KERNEL_SIGNATURE(vectorized_unbalanced) {
  // Set constants for multiply and add
  const VEC_DATA_T rv = VEC_FUNC(set1, 1.0 + 1e-6);
  const VEC_DATA_T sv = VEC_FUNC(set1, 1e-6);

  // Run kernel. Manually unrolled 8 times to have enough independent
  // instrustions in the reservation station to keep all the FMA units
  // busy. Unrolling more may result in register spilling.
#pragma omp parallel for
  for (uint64_t i = 0; i < WSS_ELEMS; i += VEC_ELEMS * 8) {
    // Load 8 array elements into registers
    VEC_DATA_T xv1 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 0)]);
    VEC_DATA_T xv2 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 1)]);
    VEC_DATA_T xv3 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 2)]);
    VEC_DATA_T xv4 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 3)]);
    VEC_DATA_T xv5 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 4)]);
    VEC_DATA_T xv6 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 5)]);
    VEC_DATA_T xv7 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 6)]);
    VEC_DATA_T xv8 = VEC_FUNC(load, &x[i + (VEC_ELEMS * 7)]);

    for (uint64_t j = 0; j < FLOPS_PER_ELEM; ++j) {
      xv1 = VEC_FUNC(mul, xv1, sv);
      xv2 = VEC_FUNC(mul, xv2, sv);
      xv3 = VEC_FUNC(mul, xv3, sv);
      xv4 = VEC_FUNC(mul, xv4, sv);
      xv5 = VEC_FUNC(mul, xv5, sv);
      xv6 = VEC_FUNC(mul, xv6, sv);
      xv7 = VEC_FUNC(mul, xv7, sv);
      xv8 = VEC_FUNC(mul, xv8, sv);
    }

    // Store the result
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 0)], xv1);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 1)], xv2);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 2)], xv3);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 3)], xv4);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 4)], xv5);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 5)], xv6);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 6)], xv7);
    VEC_FUNC(store, &x[i + (VEC_ELEMS * 7)], xv8);
  }
}

/**
 * Runners
 */
#define DEFINE_RUN_RW_ROOFLINE(optimization)                                          \
  template<size_t FLOPS_PER_ELEM>                                                     \
  res_t run_rw_roofline_##optimization (double allowed_time, data_t* a){              \
    res_t result;                                                                     \
    run_kernel(rw_roofline_##optimization <FLOPS_PER_ELEM>(a), allowed_time, result); \
    result.bytes_read = result.iters * WSS_ELEMS * sizeof(data_t);                    \
    result.bytes_write = result.bytes_read;                                           \
    return result;                                                                    \
  }

DEFINE_RUN_RW_ROOFLINE(vectorized_balanced)
DEFINE_RUN_RW_ROOFLINE(vectorized_unbalanced)

/**
 * Instantiate required kernels for profiling
 */

#define INSTANTIATE_RUN_RW_ROOFLINE(optimization, flops_per_elem)                    \
  template res_t run_rw_roofline_##optimization <flops_per_elem>(double allowed_time, data_t* a);


INSTANTIATE_RUN_RW_ROOFLINE(vectorized_balanced, 2)
INSTANTIATE_RUN_RW_ROOFLINE(vectorized_balanced, 8192)
INSTANTIATE_RUN_RW_ROOFLINE(vectorized_unbalanced, 8192)
