# sift – Rule‑Based File Organizer

**sift** is a lightweight, single‑binary C program that scans a directory (depth 1 only) and moves files into designated subfolders based on flexible matching rules.

Unlike simple extension‑based sorters, **sift** supports three match types:

- **Starts with** – e.g., `IMG_2025.png` → `img/`
- **Contains** – e.g., `backup_report.pdf` → `backups/` (uses `strstr()` under the hood)
- **Ends with** – e.g., `notes.txt` → `txt/` (classic extension sorting)

Rules are evaluated **in order** – the first rule that matches decides the destination. This gives you complete control over prioritisation.

## Features

- ✅ Depth‑1 crawling – only top‑level files are processed
- ✅ Three match types – starts, contains, ends
- ✅ Case‑sensitive matching (easily configurable)
- ✅ Destination folders created automatically
- ✅ Minimal dependencies – just standard C and POSIX
- ✅ Dry‑run mode (`-n`) to preview changes
- ✅ Fast and memory‑efficient

---

## Requirements

- **C17** compiler (gcc or clang)
- **argp** — built into glibc on Linux; install via `brew install argp-standalone` on macOS

---

## Build

```sh
make help     # show available targets
make                                # optimised release build -O3
make debug -B O_DEBUG=1             # debug build with -g3 -DDEBUG
make install                        # install to /usr/local/bin (use PREFIX= to override)
make install PREFIX="$HOME/.local"  # install to $HOME/.local
make clean
```

## Usage

```
sift [OPTION...] [TARGET(s)...]
```

### Options

| Flag          | Short | Place shoulder | Description                                                     |
| ------------- | ----- | -------------- | --------------------------------------------------------------- |
| `--dry-run`   | `-n`  | —              | Show what would change without making any changes               |
| `--log-level` | `-L`  | `info`         | `off` , `fatal` , `error` , `warn` , `info` , `debug` , `trace` |
| `--log-file`  | `-F`  | `FILE`         | Set logging file                                                |

### Examples

```sh
# See what would be synced without making changes
sift --dry-run

# Verbose debug output
sift --log-level debug
```

---

## Project structure

```
./sift
└── src/
│   ├── main.c            # CLI argument parsing, sync loop
│   ├── log.h             # LOG_ERROR / LOG_WARN / LOG_INFO / LOG_DEBUG macros
│   ├── log.c             # log_record() implementation
│   └── project_config.h  # Version, name, global rclone options
└── Makefile
```

---

## License

MIT — see [LICENSE](LICENSE).

---

## See Also

- [PROJECT_BRIEF.md](docs/PROJECT_BRIEF.md) — Architecture, module guide, mental model
