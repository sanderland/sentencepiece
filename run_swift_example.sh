#!/bin/bash

# Exit on error
set -e
set -x

# Configuration
INPUT_FILE=../pyunigram/tests/data/swift_clean.txt
MODEL_PREFIX=swift_model
VOCAB_SIZE=1024
MODEL_TYPE=unigram

# build
cd build
echo -e "\n=== Configuring build ==="
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS_DEBUG="-g -O0"

echo -e "\n=== Building SentencePiece ==="
make -j$(sysctl -n hw.ncpu) VERBOSE=1

# Go back to the root directory
cd ..

# Build the training command
TRAIN_CMD=(
  "build/src/spm_train"
  "--input=$INPUT_FILE"
  "--model_prefix=$MODEL_PREFIX"
  "--vocab_size=$VOCAB_SIZE"
  "--model_type=$MODEL_TYPE"
  "--character_coverage=1.0"
  "--byte_fallback=false"
  "--normalization_rule_name=identity"
  "--remove_extra_whitespaces=false"
)

echo -e "\n=== Training model on $INPUT_FILE ==="
if [ "$DEBUG" = "1" ]; then
  echo "Running in debug mode with lldb..."
  echo "To set a breakpoint, use: breakpoint set --file unigram_model_trainer.cc --line 347"
  echo "Then type 'run' to start the program"
  lldb -- "${TRAIN_CMD[0]}" "${TRAIN_CMD[@]:1}"
else
  "${TRAIN_CMD[@]}"
fi

echo -e "\n=== Training complete! Model files created: ==="
ls -lh "${MODEL_PREFIX}".*

echo -e "\n=== Sample of the vocabulary: ==="
head -n 20 "${MODEL_PREFIX}.vocab"

echo -e "\n=== Example encoding: ==="
echo "This is a test sentence to encode." | build/src/spm_encode --model="${MODEL_PREFIX}.model"
