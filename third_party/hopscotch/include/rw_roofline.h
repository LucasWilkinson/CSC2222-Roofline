//
// Created by lwilkinson on 10/13/21.
// Adapted from: https://github.com/alifahmed/hopscotch/blob/master/cpu/include/common.h
//

#ifndef ROOFLINE_DEMO_RW_ROOFLINE_H
#define ROOFLINE_DEMO_RW_ROOFLINE_H

#include <cstdint>
#include <common.h>

// Roofline specific kernel
template<size_t FLOPS_PER_ELEM>
res_t run_rw_roofline(double allowed_time, data_t* a);

#endif //ROOFLINE_DEMO_RW_ROOFLINE_H
