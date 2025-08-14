#!/bin/bash

# Exit on error
set -e
set -x

# Configure and build
cd build
echo -e "\n=== Configuring build ==="
cmake ..

echo -e "\n=== Building SentencePiece ==="
make -j$(nproc)

# Go back to the root directory
cd ..

echo -e "\n=== Training model on botchan.txt ==="
build/src/spm_train \
  --input=data/botchan.txt \
  --model_prefix=botchan_model \
  --vocab_size=2000 \
  --model_type=unigram

echo -e "\n=== Training complete! Model files created: ==="
ls -lh botchan_model.*

echo -e "\n=== Sample of the vocabulary: ==="
head -n 20 botchan_model.vocab

echo -e "\n=== Example encoding: ==="
echo "This is a test sentence to encode." | build/src/spm_encode --model=botchan_model.model

echo -e "\n=== Done! ==="
