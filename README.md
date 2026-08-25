# sift – Rule-Based File Organizer

**sift** is a lightweight, single-binary C program that scans a directory (depth 1 only) and moves files into designated subfolders based on flexible matching rules.

Rules are evaluated **in order** — the first rule that matches decides the destination. This gives you complete control over prioritisation.

## Features

- Depth-1 crawling — only top-level files are processed
- Three match types — starts with, contains, ends with
- Optional content sniffing (`-M`) — recognizes files by magic bytes, not just extension
- Dry-run mode (`-n`) to preview changes without touching files
- Destination folders created automatically on first match
- Collision protection — refuses to silently overwrite an existing target
- Multi-directory support via `SIFT_DIRS` environment variable
- Minimal dependencies — standard C23 + POSIX + argp

---

## Requirements

- **C23** compiler (gcc or clang)
- **argp** — built into glibc on Linux; install via `brew install argp-standalone` on macOS

---

## Build

```sh
make                            # optimised release build (-O3)
make debug                      # debug build with -g3, ASan + UBSan
make help                       # show available targets and tunables
make install                    # install to /usr/local/bin
make install PREFIX="$HOME/.local"
make clean
```

---

## Usage

```
sift [OPTION...] [DIRECTORY]
```

### Environment Variables

| Variable    | Description                                                                                                                                                                                               |
| ----------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `SIFT_DIRS` | `;`-separated list of directories to scan. Used when no explicit `DIRECTORY` argument is given. Invalid entries (non-existent paths) are skipped with a warning. Empty string (`""`) is treated as unset. |

**Precedence:** explicit `DIRECTORY` argument > `SIFT_DIRS` > current directory (`.`).

```sh
# Set once, use everywhere
export SIFT_DIRS="$HOME/Downloads;$HOME/Desktop;$HOME/Documents"
sift                          # scans all three directories

sift ~/Inbox                  # overrides SIFT_DIRS, scans only ~/Inbox

SIFT_DIRS="/tmp/data" sift    # override for a single run (no export)
```

### Options

| Flag          | Short | Default | Description                                               |
| ------------- | ----- | ------- | --------------------------------------------------------- |
| `--dry-run`   | `-n`  | off     | Show what would move without making any changes           |
| `--mime-type` | `-M`  | off     | Detect file type by content (magic bytes), not just name  |
| `--rules`     | `-R`  | off     | Print the current rule table and exit                     |
| `--log-level` | `-L`  | `info`  | `off`, `fatal`, `error`, `warn`, `info`, `debug`, `trace` |
| `--log-file`  | `-F`  | stderr  | Redirect log output to a file                             |

### Examples

```sh
# Sort files in the current directory
sift

# Sort a specific directory
sift ~/Downloads

# Preview what would move (no changes made)
sift -n ~/Downloads

# Use magic-byte content detection (recognize by file content, not extension)
sift -M ~/Downloads

# Sort multiple directories at once
SIFT_DIRS="~/Downloads;~/Desktop;~/Documents" sift

# Explicit argument overrides SIFT_DIRS
SIFT_DIRS="~/Downloads" sift ~/Desktop

# Verbose debug output
sift -L debug ~/Downloads
```

---

## Rules

Rules are evaluated first-match-wins. The built-in rule table:

| Match    | Pattern    | Destination | Notes                                                   |
| -------- | ---------- | ----------- | ------------------------------------------------------- |
| starts   | `WhatsApp` | `whatsapp/` |                                                         |
| starts   | `IMG_`     | `img/`      |                                                         |
| starts   | `DSC_`     | `photos/`   |                                                         |
| starts   | `VID_`     | `videos/`   |                                                         |
| contains | `backup`   | `backups/`  |                                                         |
| contains | `temp`     | `tmp/`      |                                                         |
| contains | `draft`    | `drafts/`   |                                                         |
| ends     | `.png`     | `img/`      | magic: PNG header                                       |
| ends     | `.jpg`     | `img/`      | magic: JPEG SOI marker                                  |
| ends     | `.jpeg`    | `img/`      |                                                         |
| ends     | `.gif`     | `img/`      | magic: GIF8                                             |
| ends     | `.bmp`     | `img/`      | magic: BM                                               |
| ends     | `.pdf`     | `doc/`      | magic: %PDF                                             |
| ends     | `.doc`     | `doc/`      | magic: OLE2 (also matches .xls/.ppt)                    |
| ends     | `.docx`    | `doc/`      | magic: ZIP local file header (also matches .xlsx/.pptx) |
| ends     | `.ppt`     | `doc/`      |                                                         |
| ends     | `.pptx`    | `doc/`      |                                                         |
| ends     | `.xls`     | `doc/`      |                                                         |
| ends     | `.xlsx`    | `doc/`      |                                                         |
| ends     | `.mp3`     | `audio/`    | magic: ID3                                              |
| ends     | `.wav`     | `audio/`    | magic: WAVE at offset 8                                 |
| ends     | `.mp4`     | `video/`    | magic: ftyp at offset 4                                 |
| ends     | `.avi`     | `video/`    | magic: AVI at offset 8                                  |

With `-M`, files that don't match by name are checked against the magic-byte signature of every rule before being skipped.

---

## Project Structure

```
sift
├── src/
│   ├── main.c              Entry point, arg dispatch, scan loop
│   ├── command_line.c/h    argp CLI parsing, SIFT_DIRS resolution
│   ├── rules.c/h           Rule table, name + magic-byte matching
│   ├── scan.c/h            Depth-1 scan, match, create folders, move
│   ├── log.c/h             Leveled logger (stderr, auto-color)
│   └── project_config.h    Version and identity strings
├── Makefile                Build recipes (release + debug)
├── .clang-format           Formatting rules (tabs, 100-col)
└── .clang-tidy             Static analysis config
```

---

## License

MIT — see [LICENSE](LICENSE).
