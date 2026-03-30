# sshbs (SSH Bootstrap)

Linux-only CLI to bootstrap SSH access on a new machine.
I'm still pretty new to C so this code might be pretty bad.

The `add` command will:
- ensure `~/.ssh` exists
- generate `~/.ssh/id_ed25519` if needed
- run `ssh-copy-id` to install your public key on the remote host
- add or update a managed alias block in `~/.ssh/config`

## Platform Support

- Linux only (for now)

## Requirements

- `ssh-keygen`
- `ssh-copy-id`

On most Linux distributions these come from `openssh-client`.

## Build

```sh
make
```

Binary output:

```sh
./bin/sshbs
```

## Install

Install into a directory that is on `PATH`.

Default install location is `/usr/local/bin`.

User-local install:

```sh
make install PREFIX=$HOME/.local
```

If `~/.local/bin` is not already on `PATH`, add this to your shell profile (`~/.bashrc` or `~/.zshrc`):

```sh
export PATH="$HOME/.local/bin:$PATH"
```

Then reload your shell:

```sh
source ~/.bashrc
```

System-wide install

```sh
sudo make install
```

Uninstall:

```sh
sudo make uninstall
```

User-local uninstall:

```sh
make uninstall PREFIX=$HOME/.local
```

## Usage

```sh
sshbs add <alias> --host <hostname> --user <username> [--port <port>] [--dry-run] [--force]
sshbs --help
```

Examples:

```sh
sshbs add app-prod --host 203.0.113.10 --user deploy
sshbs add app-staging --host staging.example.com --user ubuntu --port 2222
sshbs add app-dev --host dev.example.com --user bob --dry-run
```

After success, your alias can be used directly:

```sh
ssh app-prod
```

## Notes

- `--dry-run` prints intended actions and does not run external commands or write SSH config.
- Managed SSH config entries are bracketed with `# sshbs-begin <alias>` and `# sshbs-end <alias>`.

## Exit Codes

- `0`: success
- `1`: runtime failure (dependency missing, command failure, file write failure)
- `2`: usage error (invalid command or arguments)
