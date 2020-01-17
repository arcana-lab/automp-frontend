#include <iostream>
#include <random>
#include <thread>
#include <math.h>
#include <omp.h>

int main (int argc, char *argv[]){

  /*
   * Fetch the inputs
   */
  if (argc < 2){
    std::cerr << "USAGE: " << argv[0] << " OUTER" << std::endl;
    return 1;
  }
  auto outerIterations = atoll(argv[1]);
  #if defined(_OPENMP)
  omp_set_num_threads(std::thread::hardware_concurrency() / 2);
  #endif

  /*
   * Initialize the values
   */
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(1,100000);
  std::vector<double> values;
  for (auto k=0; k < outerIterations; k++){
    values.push_back(distribution(generator));
  }

  /*
   * Compute the values
   */
  #pragma omp parallel for shared(outerIterations, values, distribution, generator) default(none)
  for (auto i=0; i < outerIterations ; i++){
    uint64_t index, index2;

    #pragma omp critical
    index = uint64_t(distribution(generator)) % outerIterations;

    #pragma omp critical
    index2 = uint64_t(distribution(generator)) % outerIterations;

    #pragma omp critical
    values[index] = sqrt((values[index2] + 10) * 2);
  }

  /*
   * Print
   */
  double sum = 0;
  for (auto k=0; k < outerIterations; k++){
    sum += values[k];
  }
  std::cout << "Total " << sum << std::endl;

  return 0;
}
