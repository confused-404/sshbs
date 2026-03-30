#include "add.h"
#include "dispatch.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ParseResult parse_add_options(int argc, char* argv[], AddOptions* opts) {
  opts->alias = NULL;
  opts->host = NULL;
  opts->user = NULL;
  opts->port = 22;
  opts->dry_run = 0;
  opts->force = 0;

  if (argc < 1) return PARSE_ERROR;

  opts->alias = argv[0];

  for (int argi = 1; argi < argc; argi++) {
    char* curr_arg = argv[argi];

    if (strcmp(curr_arg, "--host") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Missing value for --host\n");
        return PARSE_ERROR;
      }
      opts->host = argv[argi + 1];
      argi++;
      continue;
    }

    if (strcmp(curr_arg, "--user") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Missing value for --user\n");
        return PARSE_ERROR;
      }
      opts->user = argv[argi + 1];
      argi++;
      continue;
    }

    if (strcmp(curr_arg, "--port") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Missing value for --port\n");
        return PARSE_ERROR;
      }
      opts->port = atoi(argv[argi + 1]);
      argi++;
      continue;
    }

    if (strcmp(curr_arg, "--dry-run") == 0) {
      opts->dry_run = 1;
      continue;
    }

    if (strcmp(curr_arg, "--force") == 0) {
      opts->force = 1;
      continue;
    }

    fprintf(stderr, "Unknown option: %s\n", curr_arg);
    return PARSE_ERROR;
  } 

  return PARSE_OK;
}

int add(int argc, char* argv[]) {
  AddOptions opts;
  ParseResult result = parse_add_options(argc, argv, &opts);

  if (result == PARSE_ERROR) return 1;

  return 0;
}
