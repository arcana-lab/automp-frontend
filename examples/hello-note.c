#include <stdio.h>

int main () {
  /**
   * Annotation DSL framework example
   *
   * "Note" is the name of the annotation framework
   *
   * dsl namespace    : noelle
   * dsl statement(s) : independent
   *
   */
  #pragma note noelle independent = 1
  for (int i = 0;; i++) { }

  int x;

  #pragma note noelle independent = 1
  {
  hi:
    x = 4; x++;
    char * str = "hello, world";
    printf("%s", str);
    return x - 5;
  }

  goto hi;
}
