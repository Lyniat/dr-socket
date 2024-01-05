#!/bin/bash

HAS_PYTHON=$(command -v python3 >/dev/null 2>&1 && echo "true")

if [ "$HAS_PYTHON" = "true" ]; then
  exec python3 tools/scripts/build.py "$@"
  exit 0
fi

echo "Error: Can't find Python3!"
exit 1