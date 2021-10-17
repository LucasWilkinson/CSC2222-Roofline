/******************************************************************************
 *
 * File: common.h
 * Description: Common header file.
 * 
 * Author: Alif Ahmed
 * Date: Sep 16, 2019
 *
 * Source: https://github.com/alifahmed/hopscotch
 *
 *****************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdint>
#include <cfloat>
#include <chrono>


// Data type of the working set
#ifndef data_t
#define data_t      float
#define ELEM_SIZE   4
#endif


// Working set size = (2 ^ WSS_EXP) bytes
// Default is 2 ^ 30 = 1GiB
#ifndef WSS_EXP
#define WSS_EXP         30
#endif

// Minimum set size before time is measured
// Default is 64MB
#ifndef WSS_MIN_EXP
#define WSS_MIN_EXP         26
#endif

// Working set size in bytes
#define WSS_BYTES       (1UL << WSS_EXP)


// Number of elements in working set
#define WSS_ELEMS       (WSS_BYTES / sizeof(data_t))


// macro expansion helper
#define XSTR(s) STR(s)
#define STR(s)  #s


// Data structure for results
typedef struct{
    uint64_t iters;
    double min_time;
    double max_time;
    double avg_time;
    uint64_t bytes_read;
    uint64_t bytes_write;
} res_t;



// Helper for print formatting
void print_bw_header();
void print_max_bw(const char* kernel, const res_t &result);


// Timer functions
std::chrono::high_resolution_clock::time_point get_time();
double get_duration(const std::chrono::high_resolution_clock::time_point &start);


// Allocates 4K aligned memory. Portable.
void* hs_alloc(size_t size);


// Initializes a data array with a constant value
void init_const(data_t* arr, uint64_t num_elem, const data_t val);


// Initializes an index array with [0,1,...,(num_elem-1)].
// If suffle is true, randomize the generated array.
void init_linear(uint64_t* arr, uint64_t num_elem, bool shuffle);


//init pointer chasing to array 'ptr' with hamiltonian cycle
void init_pointer_chasing(void ** ptr, uint64_t num_elem);


// Runs kernel till allowed_time is elapsed, and then stores the following results:
//     result.iters
//     result.min_time
//     result.max_time
//     result.avg_time
// Other variables of results are not changed.
#define run_kernel(kernel, allowed_time, result)                            \
        {                                                                   \
            double total_time = 0;                                          \
            double min_time = DBL_MAX;                                      \
            double max_time = 0;                                            \
            uint64_t iters = 0;                                             \
            static const uint64_t min_iter = (WSS_MIN_EXP <= WSS_EXP) ? 1 : \
                                   (1UL << WSS_MIN_EXP) / (1UL << WSS_EXP); \
                                                                            \
            kernel; /*warm up*/                                             \
            while(total_time < allowed_time) {                              \
                auto t_start = get_time();                                  \
                for(uint64_t i = 0; i < min_iter; i++) {                    \
                    kernel;                                                 \
                }                                                           \
                double t = get_duration(t_start);                           \
                if(t < min_time){                                           \
                    min_time = t;                                           \
                }                                                           \
                if(t > max_time){                                           \
                    max_time = t;                                           \
                }                                                           \
                total_time += t;                                            \
                iters += min_iter;                                          \
            }                                                               \
                                                                            \
            result.iters = iters;                                           \
            result.min_time = min_time / min_iter;                          \
            result.max_time = max_time / min_iter;                          \
            result.avg_time = total_time / iters;                           \
        }

#endif /* _COMMON_H_ */