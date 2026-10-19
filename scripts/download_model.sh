#!/bin/bash
set -e

MODEL_DIR="models"
MODEL_FILE="qwen2.5-0.5b-instruct-q8_0.gguf"
MODEL_URL="https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/${MODEL_FILE}"

mkdir -p "$MODEL_DIR"

if [ -f "$MODEL_DIR/$MODEL_FILE" ]; then
    echo "✅ Model already exists: $MODEL_DIR/$MODEL_FILE"
    ls -lh "$MODEL_DIR/$MODEL_FILE"
    exit 0
fi

echo "📥 Downloading $MODEL_FILE (~531MB)..."
if command -v wget >/dev/null 2>&1; then
    wget -O "$MODEL_DIR/$MODEL_FILE" "$MODEL_URL"
elif command -v curl >/dev/null 2>&1; then
    curl -L -o "$MODEL_DIR/$MODEL_FILE" "$MODEL_URL"
else
    echo "❌ Neither wget nor curl found"
    exit 1
fi

echo "✅ Done"
ls -lh "$MODEL_DIR/$MODEL_FILE"
