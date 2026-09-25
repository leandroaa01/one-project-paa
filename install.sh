#!/bin/bash
rm -rf build bin
cmake -S . -B build
cmake --build build