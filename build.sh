#!/bin/bash
clear

if [ ! -d ./build ]; then
    echo "Creating $BUILD_DIR"
    mkdir $BUILD_DIR
fi

echo "Building..."

time gcc \
    -g \
    ./src/main.c ./src/nuklear_imp.c \
    -std=gnu99 \
    -Wall -Wextra -Wno-missing-braces -Wno-unused-function \
    -lm -lSDL2 -lGL \
    -o ./build/nes

EC=$?

[ $EC -eq 0 ] && echo "Build succesfull" || echo "Build failed"
