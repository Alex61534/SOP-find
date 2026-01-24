# myfind

Limited `find` implementation for the Systemnahe Programmierung portfolio (Portfolio 3+4).
Focus: filesystem traversal, filters, dynamic data structures, and multithreaded execution in C.

## Build

```bash
make
```

## Run

```bash
./myfind <path> [options]
```

## Options

```
  -n, --name NAME          exact filename match
  -s, --suffix SUFFIX      file suffix (case-insensitive, without dot)
  -t, --type TYPE          file type (e.g. f, d, l)
  -c, --name-contains NAME filename contains name
  -e, --exclude NAME       filename can not contain name
      --size-min KB        minimum size in KB
      --size-max KB        maximum size in KB
      --maxdepth N         maximum recursion depth (0 = only start dir)
  -D, --delete             prompt before deleting matches (files only)
  -h, --help               show this help and exit
```

## Examples

```bash
./myfind . --name main.c
./myfind /var/log --suffix log --size-min 10
./myfind /home --type d --maxdepth 2
./myfind . --name-contains test --exclude build
```

## stdin / stdout (Pipelines)

- If no `<path>` argument is given and stdin is not a TTY, `myfind` reads paths
  line-by-line from stdin and runs the search for each path.
- All matching paths are written to stdout; errors are written to stderr.
- `--delete` requires an interactive terminal for confirmation.

## Implementation Notes

- Language: C (C11), compiled with `gcc`.
- Filesystem API: `lstat`, `opendir`, `readdir`, `unlink`.
- Dynamic data structure: a linked-list queue (`QueueItem`) used as a work queue.
- Concurrency: worker threads (2..8 based on CPU count) process directories in parallel.
- Synchronization: `pthread_mutex_t` and `pthread_cond_t` protect the shared work queue;
  a separate mutex serializes stdout/stderr output and delete confirmations.

## Project Structure

- `src/main.c`: argument handling and entry point
- `src/cli.c`: CLI parsing and usage help
- `src/walk.c`: multithreaded directory traversal
- `src/filters.c`: filter logic
- `src/queue.c`: linked-list queue
- `src/io.c`: stdin handling and path normalization
- `tests/`: black-box tests with `tests/run.sh`

## Tests

```bash
make test
```

## Makefile Targets

- `make` or `make myfind`: build the binary
- `make clean`: remove `myfind`
- `make test`: run tests (builds `myfind` first)

## Use of AI Assistance

We used GPT only minimally (e.g., wording/structure checks for the README). The code
implementation was written and reviewed by the team.
