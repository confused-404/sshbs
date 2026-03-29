#ifndef DISPATCH_H
#define DISPATCH_H

#include "add.h"

typedef enum {
  COMMAND_UNKNOWN,
  COMMAND_ADD,
  COMMAND_COUNT,
} Command; 

typedef int (*CommandFunc)(int argc, const char* argv[]);

extern const CommandFunc COMMAND_DISPATCH_TABLE[];

Command parse_command(const int argc, const char* argv[]);

#endif
