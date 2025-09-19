#!/bin/bash

set -e  # Exit on any error

echo "=== Railway Build Script ==="

# Install Python dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Install system dependencies
echo "Installing system dependencies..."
if command -v apt-get &> /dev/null; then
    echo "Using apt-get to install packages..."
    apt-get update
    apt-get install -y build-essential wget
    # Try different GLPK package names
    apt-get install -y libglpk-dev || apt-get install -y libglpk0-dev || apt-get install -y glpk-dev
    echo "Package installation completed"
elif command -v yum &> /dev/null; then
    echo "Using yum to install packages..."
    yum install -y gcc-c++ glpk-devel
elif command -v dnf &> /dev/null; then
    echo "Using dnf to install packages..."
    dnf install -y gcc-c++ glpk-devel
else
    echo "No package manager found, trying to continue..."
fi

# Try to install GLPK from source if package manager failed
echo "Checking GLPK installation..."
if [ ! -f "/usr/include/glpk.h" ] && [ ! -f "/usr/local/include/glpk.h" ]; then
    echo "GLPK not found in standard locations, trying to install from source..."
    if command -v wget &> /dev/null; then
        echo "Installing GLPK from source..."
        cd /tmp
        wget https://ftp.gnu.org/gnu/glpk/glpk-5.0.tar.gz
        tar -xzf glpk-5.0.tar.gz
        cd glpk-5.0
        ./configure --prefix=/usr/local
        make
        make install
        ldconfig
        echo "GLPK installed from source"
    else
        echo "Cannot install GLPK - wget not available"
        exit 1
    fi
else
    echo "GLPK found in standard location"
fi

# Build the C++ algorithm
echo "Building C++ algorithm..."
make web-real

echo "Build completed successfully!"
