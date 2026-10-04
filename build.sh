#!/bin/bash


set -e

echo "Building project..."
make all

echo ""
echo "Running unit tests..."
./bin/unit_tests

echo ""
echo "Running benchmark..."
./bin/benchmark

echo ""
echo "Build complete. Run ./run.sh to launch application."
