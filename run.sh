#!/usr/bin/env bash

set -e

function build() {
    cmake --build build --config Debug --target main --
}

function run() {
    ./build/main
}

TARGET=$1
case $TARGET in
build)
    build
    ;;
*)
    build
    run
    ;;
esac
