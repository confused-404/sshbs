CC = gcc

CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror \
         -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align \
         -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings \
         -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion \
         -Wunreachable-code -MMD -MP

LDFLAGS =

TARGET = sshbs
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DESTDIR ?=

BUILD_DIR = build
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)

all: bin/$(TARGET)

bin/$(TARGET): $(OBJ)
	@echo "Building sshbs..."
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete."

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all run clean install uninstall

run: bin/$(TARGET)
	./bin/$(TARGET)

clean:
	rm -rf $(BUILD_DIR) bin/$(TARGET)

install: bin/$(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 0755 bin/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

-include $(DEP)
