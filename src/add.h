#ifndef ADD_H
#define ADD_H

typedef struct {
  char* alias;
  char* host;
  char* user;
  int port;
  int dry_run;
} AddOptions;

int add(const int argc, const char* argv[]);

#endif
