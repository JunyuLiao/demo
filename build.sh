#!/bin/bash

set -e  # Exit on any error

echo "=== Railway Build Script ==="

# Debug: Show system information
echo "System information:"
uname -a
echo "Current directory: $(pwd)"
echo "Available commands:"
which gcc g++ make wget curl || echo "Some commands not found"

# Install Python dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Install system dependencies
echo "Installing system dependencies..."
if command -v apt-get &> /dev/null; then
    echo "Using apt-get to install packages..."
    apt-get update
    apt-get install -y build-essential wget curl
    # Try to install GLPK with different approaches
    echo "Attempting to install GLPK..."
    apt-get install -y libglpk-dev 2>/dev/null || \
    apt-get install -y libglpk0-dev 2>/dev/null || \
    apt-get install -y glpk-dev 2>/dev/null || \
    apt-get install -y glpk-utils 2>/dev/null || \
    echo "Package manager GLPK installation failed"
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

# Check for GLPK in various locations
echo "Searching for GLPK installation..."
find /usr -name "glpk.h" 2>/dev/null || echo "GLPK not found in /usr"
find /usr/local -name "glpk.h" 2>/dev/null || echo "GLPK not found in /usr/local"
find /opt -name "glpk.h" 2>/dev/null || echo "GLPK not found in /opt"

# Try to install GLPK from source if not found
echo "Checking GLPK installation..."
if [ ! -f "/usr/include/glpk.h" ] && [ ! -f "/usr/local/include/glpk.h" ]; then
    echo "GLPK not found in standard locations, installing from source..."
    if command -v wget &> /dev/null; then
        echo "Installing GLPK from source..."
        cd /tmp
        wget https://ftp.gnu.org/gnu/glpk/glpk-5.0.tar.gz
        tar -xzf glpk-5.0.tar.gz
        cd glpk-5.0
        ./configure --prefix=/usr/local
        make -j$(nproc)
        make install
        ldconfig
        echo "GLPK installed from source to /usr/local"
        # Update library path
        export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
        echo "Updated LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
    else
        echo "Cannot install GLPK - wget not available"
        exit 1
    fi
else
    echo "GLPK found in standard location"
fi

# Final check for GLPK
echo "Final GLPK check:"
if [ -f "/usr/include/glpk.h" ]; then
    echo "GLPK header found at /usr/include/glpk.h"
elif [ -f "/usr/local/include/glpk.h" ]; then
    echo "GLPK header found at /usr/local/include/glpk.h"
else
    echo "ERROR: GLPK header still not found!"
    echo "Available headers in /usr/include:"
    ls /usr/include/ | grep -i glpk || echo "No GLPK files found"
    exit 1
fi

# Build the C++ algorithm
echo "Building C++ algorithm..."
make web-real

echo "Build completed successfully!"
