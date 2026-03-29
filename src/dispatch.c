#include "dispatch.h"

#include <string.h>

const CommandFunc COMMAND_DISPATCH_TABLE[COMMAND_COUNT] = {
  [COMMAND_UNKNOWN] = NULL,
  [COMMAND_ADD] = add,
};

Command parse_command(int argc, char* argv[]) {
  if (argc < 2) {
    return COMMAND_UNKNOWN;
  }

  const char* str_command = argv[1];

  if (strcmp(str_command, "add") == 0) {
    return COMMAND_ADD;
  }

  return COMMAND_UNKNOWN;
}
