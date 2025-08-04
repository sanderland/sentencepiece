#!/bin/sh

mkdir -p sentencepiece

for i in CMakeLists.txt LICENSE README.md VERSION.txt cmake config.h.in  sentencepiece.pc.in src third_party
do
  echo "copying ../${i} sentencepiece/${i}"
  cp -f -R "../${i}" sentencepiece
done

mkdir -p sentencepiece/data
cp -f ../data/*.bin sentencepiece/data

python3 setup.py sdist
