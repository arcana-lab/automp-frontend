#include <stdio.h>

int main (void) {
  #pragma note noelle independent = 0
  {
    int x = 5;
    x++;
    #pragma note noelle ordered = 1
    {
      x--; x--; x--;
    }
    for (; x < 5; x++);
    #pragma note noelle ordered = 0
    {
      x--; x--; x--;
    }
    printf("%d", x);
  }
}
