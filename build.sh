#!/bin/bash

# Install system dependencies
apt-get update
apt-get install -y build-essential libglpk-dev

# Install Python dependencies
pip install -r requirements.txt

# Build the C++ algorithm
make web-real

echo "Build completed successfully!"
