# sift -- build recipe
#
# Two flavors, kept in separate trees:
#   make            release (-O3) -> build/release/sift
#   make debug      sanitized     -> build/debug/sift  (ASan + UBSan, -g3)
#
# After linking, a ./sift symlink points at the freshly built binary,
# so you can always run the last flavor you built as ./sift.
#
# Tunables (also listed by `make help`; pass on the command line):
#   make O_DEBUG=1                      force a debug-flavored build
#   make O_LOG_SHOW_SOURCE_LOCATION=1   log lines get [file:line:func]
#   make install PREFIX=$HOME/.local    relocate installation

UNAME_S := $(shell uname -s)

# Installation layout (DESTDIR supported for packaging).
PREFIX ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin
MANPREFIX ?= $(PREFIX)/share/man/man1

STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install

# NOTE: never put a trailing "# comment" on an assignment line -- make
# strips the comment but keeps the preceding blanks, silently adding
# them to the value (BUILD_TYPE would become "release          ").
BUILD_TYPE      ?= release
PROJECT_EXE_NAME = sift
BUILD            = build/$(BUILD_TYPE)
BIN              = $(BUILD)/$(PROJECT_EXE_NAME)

# HEADERS tracked for completeness; not used in rules yet.
# SRC: every .c under src/ gets compiled.
HEADERS   = $(wildcard src/*.h)
SRC       = $(wildcard src/*.c)

# Common flags
CFLAGS += -Isrc -std=c23

# Warnings are part of the definition of done: release builds must be
# warning-free. -Wno-missing-field-initializers allows tables/structs
# that intentionally leave trailing fields zeroed (e.g. G_rules).
CFLAGS +=  -Wshadow -Wconversion \
           -Wall -Wextra -Wpedantic \
           -Wno-missing-field-initializers \
           -Wstrict-prototypes -Wmissing-prototypes

# LDLIBS += -lpthread

# Build options (set via command line, e.g. `make O_DEBUG=1`)
# NOTE: the double-## comments here are load-bearing -- `make help`
# parses them to render the variable/target listing below.
O_DEBUG := 0                     ## Enable debug build (ASan, UBSan, -g3)
O_LOG_SHOW_SOURCE_LOCATION := 0  ## Prepend [file:line:fun] to log output

# `make debug` implies the debug flavor without extra variables:
# MAKECMDGOALS contains the goals given on this invocation, so we can
# see "debug" coming and flip the switches before anything compiles.
ifneq ($(filter debug,$(MAKECMDGOALS)),)
	O_DEBUG := 1
	O_LOG_SHOW_SOURCE_LOCATION := 1
endif

ifeq ($(strip $(O_DEBUG)),1)
	# Debug instrumentation: full debug info, DEBUG macro for #ifdefs,
	# source locations in log output, stack-usage reports (*.su).
	CFLAGS += -g3 -DDEBUG -DLOG_SHOW_SOURCE_LOCATION -DLOG_SHOW_FUN_NAME

	# Sanitizers must be present at BOTH compile and link time.
	LDFLAGS += -fsanitize=address -fsanitize=undefined
	CFLAGS += -fstack-usage \
	          -fsanitize=address \
	          -fsanitize=undefined

    ifneq (,$(findstring clang,$(CC)))
	CFLAGS += -ffreestanding  # clang-only debug quirk inherited from the template
    endif
else
	CFLAGS += -O3
endif

# Convert O_ variables to -D flags
ifeq ($(strip $(O_LOG_SHOW_SOURCE_LOCATION)),1)
	CFLAGS += -DLOG_SHOW_SOURCE_LOCATION
endif

# Platform-specific settings
ifeq ($(UNAME_S),Darwin)
	# macOS: argp is not in libc; provided by Homebrew's argp-standalone
	# (brew install argp-standalone). The Makefile links it here.
	LDLIBS += -largp
else
	# Linux/glibc: _GNU_SOURCE exposes POSIX/GNU extensions we rely on,
	# notably memmem() in rules.c.
	CFLAGS += -D_GNU_SOURCE
endif

# Mirror the src/ tree inside build/<flavor>/ : src/rules.c becomes
# build/release/src/rules.o, etc.
OUT = $(SRC:%.c=$(BUILD)/%.o)

all: $(BIN)

help:  ## Show this help
	@echo "Variable:"
	@awk 'BEGIN {FS="  ## "} \
		/^O_[a-zA-Z_]+[[:space:]]*:=/ { \
		split($$1, a, /[[:space:]]*:=/); \
		printf "  \033[36m%-30s\033[0m %s\n", a[1], $$2; \
	}' $(MAKEFILE_LIST)

	@echo
	@echo "Targets:"
	@grep -hE '^[a-zA-Z_-]+:.*  ## ' $(MAKEFILE_LIST) | \
	awk 'BEGIN {FS="  ## "}; {printf "  \033[33m%-15s\033[0m %s\n", $$1, $$2}'

$(BUILD):  ## Create build directories automatically
	mkdir -p $(BUILD)

# Order-only-ish directory creation: ensure the object's folder exists
# before compiling into it.
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OUT)  ## Build the sift binary
	$(CC) $(LDFLAGS) -o $@ $(OUT) $(LDLIBS)
	ln -sf $(BIN) $(PROJECT_EXE_NAME)  # convenience: run as ./sift

# Recursive invocation: a fresh make with BUILD_TYPE=debug re-evaluates
# everything above into build/debug/, keeping the two flavors' objects
# out of each other's way.
debug:  ## Build the debug binary with `make debug O_DEBUG=1`
	$(MAKE) BUILD_TYPE="debug" O_DEBUG=1

# Depends on strip: the installed binary is the stripped one. Side
# effect -- the local build tree's binary is stripped too.
install: strip  ## Install the sift binary
	$(INSTALL) -m 0755 -d $(DESTDIR)$(BINPREFIX)
	$(INSTALL) -m 0755 $(BIN) $(DESTDIR)$(BINPREFIX)

	$(INSTALL) -m 0755 -d $(DESTDIR)$(MANPREFIX)
	$(INSTALL) -m 0755 sift.1 $(DESTDIR)$(MANPREFIX)

clean:  ## Clean up build artifacts
	$(RM) -rf $(OUT) $(BIN)  # removes objects + binaries, keeps build/ dirs

uninstall: ## Uninstall the sift binary
	$(RM) $(DESTDIR)$(BINPREFIX)/$(BIN)
	$(RM) $(DESTDIR)$(MANPREFIX)/sift.1

strip: $(BIN)  ## Strip the sift binary
	$(STRIP) $^

.PHONY: all install uninstall strip clean debug
