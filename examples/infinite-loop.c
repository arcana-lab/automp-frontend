void loop (int value) {
  while (1) {
    value++;
  }
}

/*
 * NOTE(Simone): By the C standard, the return type of main must be `int`
 * and if a `return <int>` is not explicitly given `return 0` will be
 * implicitly inserted before the terminating `}`. See C99 5.1.2.2.3.
 *
 * Unfortunately, this creates a horrible-ugly disconnected control-flow
 * graph, making it inadvisable (at the very least) to loop infinitely
 * from `main(...)`. While the compiler may happily compile it, the CFG,
 * as mentioned, is garbage due to the unreachable node(s).
 *
 */
int main (int argc, char * argv[]) {
  loop(argc);
  return 0;
}
