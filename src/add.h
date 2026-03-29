#ifndef ADD_H
#define ADD_H

typedef struct {
  char* alias;
  char* host;
  char* user;
  int port;
  int dry_run;
  int force;
} AddOptions;

AddOptions parse_add_options(int argc, char* argv[]);
int add(int argc, char* argv[]);

#endif
