#ifndef DISPATCH_H
#define DISPATCH_H

typedef enum {
  COMMAND_UNKNOWN,
  COMMAND_ADD,
  COMMAND_COUNT,
} Command; 

typedef enum {
  PARSE_OK,
  PARSE_ERROR,
} ParseResult;

typedef int (*CommandFunc)(int argc, char* argv[]);

extern const CommandFunc COMMAND_DISPATCH_TABLE[];

Command parse_command(const int argc, char* argv[]);

#endif
