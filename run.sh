#!/bin/bash
# Simple build and run script for the OpenGL project

set -e  # Exit on error

echo "Building project..."
cmake --build build

echo "Running application..."
cd "$(dirname "$0")"
./build/my_app
