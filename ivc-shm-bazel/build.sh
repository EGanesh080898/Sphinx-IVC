#!/bin/bash

BUILD_DIR=./build

if [ -d "$BUILD_DIR" ]; then
  rm -rf $BUILD_DIR/* 
else
  mkdir $BUILD_DIR
fi
cmake -B$BUILD_DIR -S.
make -j$(nproc) -C $BUILD_DIR
