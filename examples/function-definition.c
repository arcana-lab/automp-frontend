#include <stdio.h>

void this_function_is_annotated (void);
void this_function_is_not (void);

#pragma note noelle independent = 1 ordered = 0
void this_function_is_annotated (void) {
  for (int x = 0; x < 100; x++) {
    printf("\n%d: iteration is independent and unordered", x);
  }

  printf("\n\n");

  #pragma note noelle ordered = 1
  for (int y = 0; y < 100; y++) {
    printf("\n%d: iteration is ordered", y);
  }

  return;
}

void this_function_is_not (void) {
  return;
}

int main (void) {
  this_function_is_annotated();
  this_function_is_not();
  return 0;
}
