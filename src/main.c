#include "dispatch.h"

#include <stdio.h>

int main(int argc, char* argv[]) {
  Command c = parse_command(argc, argv);

  if (c == COMMAND_UNKNOWN) {
    fprintf(stderr, "Invalid command\n");
    return 1;
  }

  int res = COMMAND_DISPATCH_TABLE[c](argc - 2, argv + 2);
  return res;
}
