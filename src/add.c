#include "add.h"

#include <string.h>

AddOptions parse_add_options(int argc, char* argv[]) {
  AddOptions opts;
  opts.port = 22;
  opts.dry_run = 0;
  opts.force = 0;

  opts.alias = argv[0];

  for (int argi = 1; argi < argc; argi++) {
    char* curr_arg = argv[argi];
    if (argi != argc - 1) {
      if (strcmp(curr_arg, "--host") == 0) {
        opts.host = curr_arg;
        argi++;
        continue;
      } 
      if (strcmp(curr_arg, "--user") == 0) {
        opts.user = curr_arg;
        argi++;
        continue;
      } 
      if (strcmp(curr_arg, "--port") == 0) {
        opts.port = (int) curr_arg;
        argi++;
        continue;
      } 
    }

    if (strcmp(curr_arg, "--dry-run") == 0) {
      opts.dry_run = 1;
      continue;
    }
    if (strcmp(curr_arg, "--force")) {
      opts.force = 1;
    }
  } 

  return opts;
}

int add(const int argc, char* argv[]) {
  AddOptions opts = parse_add_options(argc, argv);
}
