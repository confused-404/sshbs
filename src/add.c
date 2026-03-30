#include "add.h"
#include "dispatch.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void print_add_usage(void) {
  printf("Usage: sshbs add <alias> --host <hostname> --user <username> [--port <port>] [--dry-run] [--force]\n");
}

static int command_exists(const char* command_name) {
  const char* path_env = getenv("PATH");
  if (!path_env || !command_name || !command_name[0]) {
    return 0;
  }

  size_t path_len = strlen(path_env);
  char* path_copy = malloc(path_len + 1U);
  if (!path_copy) {
    return 0;
  }

  memcpy(path_copy, path_env, path_len + 1U);

  int found = 0;
  char* dir = strtok(path_copy, ":");
  while (dir) {
    char candidate[1024];
    int written = snprintf(candidate, sizeof(candidate), "%s/%s", dir, command_name);
    if (written > 0 && (size_t)written < sizeof(candidate) && access(candidate, X_OK) == 0) {
      found = 1;
      break;
    }
    dir = strtok(NULL, ":");
  }

  free(path_copy);
  return found;
}

static int ensure_ssh_dir(const char* home, const int dry_run) {
  char ssh_dir[1024];
  struct stat st;

  int written = snprintf(ssh_dir, sizeof(ssh_dir), "%s/.ssh", home);
  if (written <= 0 || (size_t)written >= sizeof(ssh_dir)) {
    fprintf(stderr, "Failed to build .ssh directory path\n");
    return 1;
  }

  if (stat(ssh_dir, &st) == 0) {
    if (!S_ISDIR(st.st_mode)) {
      fprintf(stderr, "Path exists but is not a directory: %s\n", ssh_dir);
      return 1;
    }
    return 0;
  }

  if (dry_run) {
    printf("Would create directory: %s\n", ssh_dir);
    return 0;
  }

  if (mkdir(ssh_dir, 0700) != 0) {
    perror("mkdir");
    return 1;
  }

  return 0;
}

static int update_ssh_config(const AddOptions* opts, const char* home) {
  char config_path[1024];
  char tmp_path[1024];
  int config_written = snprintf(config_path, sizeof(config_path), "%s/.ssh/config", home);
  int tmp_written = snprintf(tmp_path, sizeof(tmp_path), "%s/.ssh/config.tmp", home);
  if (config_written <= 0 || (size_t)config_written >= sizeof(config_path) ||
      tmp_written <= 0 || (size_t)tmp_written >= sizeof(tmp_path)) {
    fprintf(stderr, "Failed to build SSH config path\n");
    return 1;
  }

  char begin_marker[512];
  char end_marker[512];
  int begin_written = snprintf(begin_marker, sizeof(begin_marker), "# sshbs-begin %s\n", opts->alias);
  int end_written = snprintf(end_marker, sizeof(end_marker), "# sshbs-end %s\n", opts->alias);
  if (begin_written <= 0 || (size_t)begin_written >= sizeof(begin_marker) ||
      end_written <= 0 || (size_t)end_written >= sizeof(end_marker)) {
    fprintf(stderr, "Alias is too long for SSH config markers\n");
    return 1;
  }

  FILE* in = fopen(config_path, "r");
  FILE* out = fopen(tmp_path, "w");
  if (!out) {
    perror("fopen");
    if (in) {
      fclose(in);
    }
    return 1;
  }

  if (in) {
    char line[1024];
    int skipping_block = 0;

    while (fgets(line, sizeof(line), in)) {
      if (!skipping_block && strcmp(line, begin_marker) == 0) {
        skipping_block = 1;
        continue;
      }

      if (skipping_block && strcmp(line, end_marker) == 0) {
        skipping_block = 0;
        continue;
      }

      if (!skipping_block && fputs(line, out) == EOF) {
        perror("fputs");
        fclose(in);
        fclose(out);
        unlink(tmp_path);
        return 1;
      }
    }

    fclose(in);
  }

  if (fputc('\n', out) == EOF ||
      fputs(begin_marker, out) == EOF ||
      fprintf(out, "Host %s\n", opts->alias) < 0 ||
      fprintf(out, "  HostName %s\n", opts->host) < 0 ||
      fprintf(out, "  User %s\n", opts->user) < 0 ||
      fprintf(out, "  Port %d\n", opts->port) < 0 ||
      fputs("  IdentityFile ~/.ssh/id_ed25519\n", out) == EOF ||
      fputs(end_marker, out) == EOF) {
    perror("write");
    fclose(out);
    unlink(tmp_path);
    return 1;
  }

  if (fclose(out) != 0) {
    perror("fclose");
    unlink(tmp_path);
    return 1;
  }

  if (rename(tmp_path, config_path) != 0) {
    perror("rename");
    unlink(tmp_path);
    return 1;
  }

  if (chmod(config_path, 0600) != 0) {
    perror("chmod");
    return 1;
  }

  return 0;
}

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
  if (argc >= 1 && (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0)) {
    print_add_usage();
    return 0;
  }

  AddOptions opts;
  if (parse_add_options(argc, argv, &opts) == PARSE_ERROR) {
    print_add_usage();
    return 1;
  }

  char path[512];
  const char* home = getenv("HOME");

  if (!home) {
    fprintf(stderr, "Could not get HOME directory\n");
    return 1;
  }

  if (!command_exists("ssh-keygen")) {
    fprintf(stderr, "Missing dependency: ssh-keygen\n");
    return 1;
  }

  if (!command_exists("ssh-copy-id")) {
    fprintf(stderr, "Missing dependency: ssh-copy-id\n");
    return 1;
  }

  if (ensure_ssh_dir(home, opts.dry_run) != 0) {
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

  if (opts.dry_run) {
    printf("Would update ~/.ssh/config for alias %s\n", opts.alias);
  } else {
    if (update_ssh_config(&opts, home) != 0) {
      fprintf(stderr, "Failed to update ~/.ssh/config\n");
      return 1;
    }
    printf("Updated ~/.ssh/config for alias %s\n", opts.alias);
  }

  printf("SSH setup complete for %s\n", target);

  return 0;
}
