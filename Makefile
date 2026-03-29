CC = gcc

CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror \
         -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align \
         -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings \
         -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion \
         -Wunreachable-code -MMD -MP

LDFLAGS =

TARGET = sshbs
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

all: bin/$(TARGET)

bin/$(TARGET): $(OBJ)
	@echo "Building sshbs..."
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete."

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all run clean

run: bin/$(TARGET)
	./bin/$(TARGET)

clean:
	rm -f src/*.o src/*.d bin/$(TARGET)

-include $(DEP)
