UNAME_S := $(shell uname -s)

PREFIX ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin
MANPREFIX ?= $(PREFIX)/share/man/man1

STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install

BUILD_TYPE      ?= release
PROJECT_EXE_NAME = sift
BUILD            = build/$(BUILD_TYPE)
BIN              = $(BUILD)/$(PROJECT_EXE_NAME)

HEADERS   = $(wildcard src/*.h)
SRC       = $(wildcard src/*.c)

# Common flags
CFLAGS += -Isrc -std=c23

CFLAGS +=  -Wshadow -Wconversion \
           -Wall -Wextra -Wpedantic \
           -Wno-missing-field-initializers \
           -Wstrict-prototypes -Wmissing-prototypes

# LDLIBS += -lpthread

# Build options (set via command line, e.g. `make O_DEBUG=1`)
O_DEBUG := 0                     ## Enable debug build (ASan, UBSan, -g3)
O_LOG_SHOW_SOURCE_LOCATION := 0  ## Prepend [file:line:fun] to log output

# Auto-enable flags for debug builds
ifneq ($(filter debug,$(MAKECMDGOALS)),)
	O_DEBUG := 1
	O_LOG_SHOW_SOURCE_LOCATION := 1
endif

ifeq ($(strip $(O_DEBUG)),1)
	CFLAGS += -g3 -DDEBUG -DLOG_SHOW_SOURCE_LOCATION -DLOG_SHOW_FUN_NAME

	LDFLAGS += -fsanitize=address -fsanitize=undefined
	CFLAGS += -fstack-usage \
	          -fsanitize=address \
	          -fsanitize=undefined

    ifneq (,$(findstring clang,$(CC)))
		CFLAGS += -ffreestanding
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
	# macOS: need argp from Homebrew (brew install argp-standalone)
	LDLIBS += -largp
else
	# Linux: _GNU_SOURCE for strptime, etc.
	CFLAGS += -D_GNU_SOURCE
endif

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

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OUT)  ## Build the sift binary
	$(CC) $(LDFLAGS) -o $@ $(OUT) $(LDLIBS)
	ln -sf $(BIN) $(PROJECT_EXE_NAME)

debug:  ## Build the debug binary with `make debug O_DEBUG=1`
	$(MAKE) BUILD_TYPE="debug" O_DEBUG=1

install: strip  ## Install the sift binary
	$(INSTALL) -m 0755 -d $(DESTDIR)$(BINPREFIX)
	$(INSTALL) -m 0755 $(BIN) $(DESTDIR)$(BINPREFIX)

	$(INSTALL) -m 0755 -d $(DESTDIR)$(MANPREFIX)
	$(INSTALL) -m 0755 sift.1 $(DESTDIR)$(MANPREFIX)

clean:  ## Clean up build artifacts
	$(RM) -rf $(OUT) $(BIN)

uninstall: ## Uninstall the sift binary
	$(RM) $(DESTDIR)$(BINPREFIX)/$(BIN)
	$(RM) $(DESTDIR)$(MANPREFIX)/sift.1

strip: $(BIN)  ## Strip the sift binary
	$(STRIP) $^

.PHONY: all install uninstall strip clean debug
