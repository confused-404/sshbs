#include "dispatch.h"

#include <stdio.h>
#include <string.h>

static void print_main_usage(void) {
  printf("sshbs - Linux SSH bootstrap helper\n");
  printf("\n");
  printf("Usage:\n");
  printf("  sshbs add <alias> --host <hostname> --user <username> [--port <port>] [--dry-run] [--force]\n");
  printf("  sshbs --help\n");
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_main_usage();
    return 2;
  }

  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
    print_main_usage();
    return 0;
  }

  Command c = parse_command(argc, argv);

  if (c == COMMAND_UNKNOWN) {
    fprintf(stderr, "Invalid command: %s\n", argv[1]);
    print_main_usage();
    return 2;
  }

  int res = COMMAND_DISPATCH_TABLE[c](argc - 2, argv + 2);
  return res;
}
