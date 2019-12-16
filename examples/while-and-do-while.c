#include <stdio.h>

int main (void) {
  int x = 5;

  #pragma note noelle independent = 1
  while (x < 10) {
    x++;
    if (x == 7) {
      break;
    }
  }

  #pragma note noelle ordered = 0 hello = world
  do {
    x--;
    if (x == 7) {
      printf("Back at %d yeeeeah", x);
    }
  } while (x > 0);

  return 0;
}
