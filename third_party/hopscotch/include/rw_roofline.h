//
// Created by lwilkinson on 10/13/21.
// Adapted from: https://github.com/alifahmed/hopscotch/blob/master/cpu/include/common.h
//

#ifndef ROOFLINE_DEMO_RW_ROOFLINE_H
#define ROOFLINE_DEMO_RW_ROOFLINE_H

#include <cstdint>
#include <common.h>

#define DECLARE_RUN_RW_ROOFLINE(optimization) \
  template<size_t FLOPS_PER_ELEM> res_t run_rw_roofline_##optimization (double allowed_time, data_t* a);

DECLARE_RUN_RW_ROOFLINE(vectorized_balanced)
DECLARE_RUN_RW_ROOFLINE(vectorized_unbalanced)

#endif //ROOFLINE_DEMO_RW_ROOFLINE_H
