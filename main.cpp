#include <iostream>

#include <graphs.hpp>
#include <rw_roofline.h>

/*******************************************************************
 *  Micro Benchmarks
 ******************************************************************/

double estimate_memory_roofline() {
  // Low AI (2 / sizeof(data_t)) ensure the kernel is memory bound so we can get an estimate of the memory bandwidth
  const size_t FLOPS_PER_ELEM = 2;

  data_t* a = (data_t*)hs_alloc(WSS_BYTES);
  init_const(a, WSS_ELEMS, 1.0);
  res_t result = run_rw_roofline_vectorized_balanced<FLOPS_PER_ELEM>(/* allowed time: */ 5, a);
  free(a);

  return (WSS_BYTES * 2.0) / result.min_time / 1e9;
}

double estimate_max_flops_roofline_balanced_vectorized() {
  // Very High AI (8192 / sizeof(data_t)) ensure the kernel is compute bound so we can get an estimate of the max flops
  const size_t FLOPS_PER_ELEM = 8192;

  data_t* a = (data_t*)hs_alloc(WSS_BYTES);
  init_const(a, WSS_ELEMS, 1.0);
  res_t result = run_rw_roofline_vectorized_balanced<FLOPS_PER_ELEM>(/* allowed time: */ 5, a);
  free(a);

  return (WSS_ELEMS * FLOPS_PER_ELEM) / result.min_time / 1e9;
}

double estimate_max_flops_roofline_unbalanced_vectorized() {
  // Very High AI (8192 / sizeof(data_t)) ensure the kernel is compute bound so we can get an estimate of the max flops
  const size_t FLOPS_PER_ELEM = 8192;

  data_t* a = (data_t*)hs_alloc(WSS_BYTES);
  init_const(a, WSS_ELEMS, 1.0);
  res_t result = run_rw_roofline_vectorized_unbalanced<FLOPS_PER_ELEM>(/* allowed time: */ 5, a);
  free(a);

  return (WSS_ELEMS * FLOPS_PER_ELEM) / result.min_time / 1e9;
}


/*******************************************************************
 *  Plotting
 ******************************************************************/

double _max_flops_estimate;
static double compute_bound(double x) { return  _max_flops_estimate; }

double _unbalanced_max_flops_estimate;
static double unbalanced_compute_bound(double x) { return  _unbalanced_max_flops_estimate; }

double _memory_bandwidth_estimate;
double memory_bound(double x) { return _memory_bandwidth_estimate * x; }

void plot_roofline(double memory_bandwidth_estimate, double max_flops_estimate) {
  size_t height = 80;
  size_t width = 160;

  double y_height = max_flops_estimate * 1.5;

  long double xmin = -20;
  long double xmax = 140;
  long double ymin = -0.1 * y_height;
  long double ymax = y_height;

  size_t numfunctions = 2;

  // Hacky but just use globals to get around function pointer passing
  _max_flops_estimate = max_flops_estimate;
  _memory_bandwidth_estimate = memory_bandwidth_estimate;

  double (*functions[])(double) = {memory_bound, compute_bound};
  graph(height, width, xmin, xmax, ymin, ymax, numfunctions, functions, graphdefaultoptions);
}

void plot_roofline(double memory_bandwidth_estimate, double max_flops_estimate, double unbalanced_max_flops_estimate) {
  size_t height = 80;
  size_t width = 160;

  double y_height = max_flops_estimate * 1.5;

  long double xmin = -20;
  long double xmax = 140;
  long double ymin = -0.1 * y_height;
  long double ymax = y_height;

  size_t numfunctions = 3;

  // Hacky but just use globals to get around function pointer passing
  _max_flops_estimate = max_flops_estimate;
  _memory_bandwidth_estimate = memory_bandwidth_estimate;
  _unbalanced_max_flops_estimate = unbalanced_max_flops_estimate;

  double (*functions[])(double) = {memory_bound, compute_bound, unbalanced_compute_bound};
  graph(height, width, xmin, xmax, ymin, ymax, numfunctions, functions, graphdefaultoptions);
}

/*******************************************************************
 *  Main
 ******************************************************************/

void wait_for_keypress() {
  do { std::cout << '\n' << "Press a key to continue..."; } while (std::cin.get() != '\n');
}

int main() {
  std::cout << "Welcome to the roofline demo!" << std::endl;
  std::cout << "We will start by profiling the performance of your machine" << std::endl;

  wait_for_keypress();

  std::cout << std::endl;
  std::cout << "Running memory benchmark..." << std::endl;
  double memory_bandwidth_estimate = estimate_memory_roofline();
  std::cout << "Bandwidth estimate: " << memory_bandwidth_estimate << " (GB/s)" << std::endl;

  std::cout << "Running throughput benchmark..." << std::endl;
  double max_flops_estimate = estimate_max_flops_roofline_balanced_vectorized();
  std::cout << "Max throughput estimate: " << max_flops_estimate << " (GFLOP/s)" << std::endl;
  std::cout << std::endl << std::endl;

  plot_roofline(memory_bandwidth_estimate, max_flops_estimate);

  wait_for_keypress();

  std::cout << std::endl;

  std::cout << "Running unbalanced mat/add throughput benchmark..." << std::endl;
  double unbalanced_max_flops_estimate = estimate_max_flops_roofline_unbalanced_vectorized();
  std::cout << "Max throughput estimate: " << unbalanced_max_flops_estimate << " (GFLOP/s)" << std::endl;
  std::cout << std::endl << std::endl;

  plot_roofline(memory_bandwidth_estimate, max_flops_estimate, unbalanced_max_flops_estimate);

  return 0;
}
