#!/bin/bash

cmake -S. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug -j 8