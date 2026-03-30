#include "add.h"
#include "dispatch.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

static int wait_for_child(pid_t pid) {
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return 1;
  }

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }

  return 1;
}

static int run_ssh_keygen(const char* key_path) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    execlp("ssh-keygen",
           "ssh-keygen",
           "-t",
           "ed25519",
           "-f",
           key_path,
           "-N",
           "",
           (char*)NULL);
    perror("ssh-keygen");
    _exit(127);
  }

  return wait_for_child(pid);
}

static int run_ssh_copy_id(const int port, const char* target) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    if (port != 22) {
      char port_str[16];
      snprintf(port_str, sizeof(port_str), "%d", port);
      execlp("ssh-copy-id",
             "ssh-copy-id",
             "-p",
             port_str,
             target,
             (char*)NULL);
    } else {
      execlp("ssh-copy-id",
             "ssh-copy-id",
             target,
             (char*)NULL);
    }

    perror("ssh-copy-id");
    _exit(127);
  }

  return wait_for_child(pid);
}

static ParseResult parse_port_value(const char* port_str, int* out_port) {
  char* end_ptr = NULL;
  errno = 0;

  long port_long = strtol(port_str, &end_ptr, 10);
  if (end_ptr == port_str || *end_ptr != '\0' || errno != 0 ||
      port_long < 1L || port_long > 65535L) {
    fprintf(stderr, "Invalid port: %s (expected 1-65535)\n", port_str);
    return PARSE_ERROR;
  }

  *out_port = (int)port_long;
  return PARSE_OK;
}

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
      if (parse_port_value(argv[argi + 1], &opts->port) == PARSE_ERROR) {
        return PARSE_ERROR;
      }
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

  char path[512];
  const char* home = getenv("HOME");

  if (!home) {
    fprintf(stderr, "Could not get HOME directory\n");
    return 1;
  }

  snprintf(path, sizeof(path), "%s/.ssh/id_ed25519", home);

  char target[256];
  snprintf(target, sizeof(target), "%s@%s", opts.user, opts.host);

  if (!opts.force) {
    printf("Generating SSH key (if not exists)...\n");
  } else {
    printf("Generating SSH key..");
  }

  int key_exists = access(path, F_OK) == 0;

  if (!opts.dry_run && (opts.force || !key_exists)) {
    int ret = run_ssh_keygen(path);
    if (ret != 0) {
      fprintf(stderr, "Failed to generate SSH key\n");
      return 1;
    }
  }

  if (opts.port != 22) {
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", opts.port);
    printf("Running: ssh-copy-id -p %s %s\n", port_str, target);
  } else {
    printf("Running: ssh-copy-id %s\n", target);
  }

  if (!opts.dry_run) {
    int ret = run_ssh_copy_id(opts.port, target);
    if (ret != 0) {
      fprintf(stderr, "Failed to copy SSH key\n");
      return 1;
    }
  }

  printf("SSH setup complete for %s\n", target);

  return 0;
}
