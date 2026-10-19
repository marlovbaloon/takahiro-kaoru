#!/bin/bash
set -e

LLAMA_DIR="third_party/llama.cpp"
LLAMA_URL="https://github.com/ggerganov/llama.cpp"

if [ -d "$LLAMA_DIR/.git" ]; then
    echo "✅ llama.cpp already cloned at $LLAMA_DIR"
    cd "$LLAMA_DIR" && git log --oneline -1 && cd - >/dev/null
    exit 0
fi

echo "📥 Cloning llama.cpp (depth=1)..."
mkdir -p third_party
git clone --depth 1 "$LLAMA_URL" "$LLAMA_DIR"

echo "✅ Done"
cd "$LLAMA_DIR" && git log --oneline -1 && cd - >/dev/null
