#!/bin/bash

cmake -S. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-debug -j 8