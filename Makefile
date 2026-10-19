 # Makefile — Kaoru × llama.cpp
CC      = clang
CFLAGS  = -O2 -g -Wall -Wextra -Iinclude \
          -Ithird_party/llama.cpp/include \
          -Ithird_party/llama.cpp/ggml/include
LDLIBS  = -Lthird_party/llama.cpp/build/bin \
          -lllama -lggml -lggml-base -lm -lpthread

SRC = src/main.c src/connectome.c src/wisdom.c src/persistence.c \
      src/memory.c src/feedback.c src/llm_bridge.c
OBJ = $(SRC:.c=.o)
BIN = kaoru

.PHONY: all clean llama run check

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

%.o: %.c include/connectome.h include/llm_bridge.h include/feedback.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clone + build llama.cpp (run once)
llama: third_party/llama.cpp
	cmake -S third_party/llama.cpp -B third_party/llama.cpp/build \
	      -DLLAMA_CURL=OFF -DGGML_NATIVE=ON -DCMAKE_BUILD_TYPE=Release
	cmake --build third_party/llama.cpp/build -j$(nproc)

third_party/llama.cpp:
	git clone --depth 1 https://github.com/ggerganov/llama.cpp third_party/llama.cpp

# Download model (run once, ~530MB)
model:
	mkdir -p models
	wget -O models/qwen2.5-0.5b-instruct-q8_0.gguf \
	  "https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/qwen2.5-0.5b-instruct-q8_0.gguf"

run: $(BIN)
	./$(BIN) models/qwen2.5-0.5b-instruct-q8_0.gguf 4

# Quick compile check (no llama.cpp needed)
check:
	$(CC) $(CFLAGS) -fsyntax-only src/connectome.c src/wisdom.c \
	      src/persistence.c src/memory.c src/feedback.c

clean:
	rm -f $(OBJ) $(BIN)