#ifndef ADD_H
#define ADD_H

#include "dispatch.h"

typedef struct {
  char* alias;
  char* host;
  char* user;
  int port;
  int dry_run;
  int force;
} AddOptions;

ParseResult parse_add_options(int argc, char* argv[], AddOptions* opts);
int add(int argc, char* argv[]);

#endif
