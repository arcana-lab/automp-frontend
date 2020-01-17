#include <iostream>
#include <random>
#include <thread>
#include <math.h>
#include <omp.h>

int main (int argc, char *argv[]){

  /*
   * Fetch the inputs
   */
  if (argc < 4){
    std::cerr << "USAGE: " << argv[0] << " OUTER MIDDLE INNER" << std::endl;
    return 1;
  }
  auto outerIterations = atoll(argv[1]);
  auto middleIterations = atoll(argv[2]);
  auto innerIterations = atoll(argv[3]);
  #if defined(_OPENMP)
  omp_set_num_threads(std::thread::hardware_concurrency() / 2);
  #endif

  /*
   * Initialize the values
   */
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(1,100000);
  std::vector<double> values;
  for (auto k=0; k < innerIterations; k++){
    values.push_back(distribution(generator));
  }

  /*
   * Compute the values
   */
  for (auto i=0; i < outerIterations ; i++){

    //#pragma omp parallel for private(middleIterations,innerIterations) shared(values) default(none)
    #pragma omp parallel for shared(values, middleIterations, innerIterations) default(none)
    for (auto j=0; j < middleIterations; j++){

      for (auto k=0; k < innerIterations; k++){
        values[k] = sqrt((values[k] + 10) * 2);
      }

    }
  }

  /*
   * Print
   */
  for (auto k=0; k < innerIterations; k++){
    std::cout << values[k] << " " ;
  }
  std::cout << std::endl;

  return 0;
}
