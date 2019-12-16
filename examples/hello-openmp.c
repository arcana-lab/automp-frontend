#include <omp.h>
#include <stdio.h>

int main () {
  // #pragma note noelle independent = 1
  #pragma omp parallel for
  for (int i = 0; i < 10; i++) { }

  // #pragma note noelle indepenent = 1
  #pragma omp parallel
  {
    int ID = omp_get_thread_num();
    printf("Hello (%i)", ID);
    printf("OpenMP (%i)\n", ID);
  }

  printf("This is printing in the middle or... implicit barrier exists?");

  return 0;
}
