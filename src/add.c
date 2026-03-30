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

  if (!opts->host) {
    fprintf(stderr, "--host is required\n");
    return PARSE_ERROR;
  }

  if (!opts->user) {
    fprintf(stderr, "--user is required\n");
    return PARSE_ERROR;
  }

  return PARSE_OK;
}

int add(int argc, char* argv[]) {
  AddOptions opts;
  if (parse_add_options(argc, argv, &opts) == PARSE_ERROR) {
    return 1;
  }

  char target[256];
  snprintf(target, sizeof(target), "%s@%s", opts.user, opts.host);

  if (!opts.force) {
    printf("Generating SSH key (if not exists)...\n");
  }

  if (!opts.dry_run) {
    int ret = system("ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519 -N \"\"");
    if (ret != 0) {
      fprintf(stderr, "Failed to generate SSH key\n");
      return 1;
    }
  }

  char cmd[512];
  if (opts.port != 22) {
    snprintf(cmd, sizeof(cmd),
             "ssh-copy-id -p %d %s",
             opts.port, target);
  } else {
    snprintf(cmd, sizeof(cmd),
             "ssh-copy-id %s",
             target);
  }

  printf("Running: %s\n", cmd);

  if (!opts.dry_run) {
    int ret = system(cmd);
    if (ret != 0) {
      fprintf(stderr, "Failed to copy SSH key\n");
      return 1;
    }
  }

  printf("SSH setup complete for %s\n", target);

  return 0;
}
