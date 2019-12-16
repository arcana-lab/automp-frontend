#include <stdio.h>

int main (int argc) {
  int break_stop = argc;

  int x;

  #pragma note noelle independent = 1
  for (x = 0; x < 100; x++) {
    if (x % 10 == 0) continue;
    if (x == break_stop) break;
  }

  x /= 10;
  printf("Reached %d 10s of iterations", x);

  return 0;
}
