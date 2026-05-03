#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
if [[ ! -f build/CMakeCache.txt ]]; then
  echo "Run cmake -B build (after installing dependencies) first." >&2
  exit 1
fi
exec cmake --build build --target run
