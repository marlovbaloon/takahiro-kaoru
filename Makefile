# Makefile — Kaoru × llama-server (HTTP)
CC      = clang
CFLAGS  = -O2 -g -Wall -Wextra -Iinclude
LDLIBS  = -lcurl -lm

SRC = src/main.c src/connectome.c src/wisdom.c src/persistence.c \
      src/memory.c src/feedback.c src/llm_bridge.c
OBJ = $(SRC:.c=.o)
BIN = kaoru

.PHONY: all clean run check

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

%.o: %.c include/connectome.h include/llm_bridge.h include/feedback.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) http://localhost:8080 4

check:
	$(CC) $(CFLAGS) -fsyntax-only src/connectome.c src/wisdom.c \
	      src/persistence.c src/memory.c src/feedback.c src/llm_bridge.c

clean:
	rm -f $(OBJ) $(BIN)