#!/bin/bash

set -e

echo "======================================"
echo "Unified Crypto Interface Build Script"
echo "======================================"
echo ""

BUILD_LIBOQS=${BUILD_LIBOQS:-1}
BUILD_GMSSL=${BUILD_GMSSL:-1}
PROJECT_ROOT=$(pwd)

if ! command -v openssl >/dev/null 2>&1; then
    echo "Error: OpenSSL command not found."
    echo "Please install OpenSSL and its development package before building UCI."
    echo "Examples:"
    echo "  Ubuntu/Debian: sudo apt install openssl libssl-dev"
    echo "  CentOS/RHEL:   sudo yum install openssl openssl-devel"
    echo "  macOS:         brew install openssl@3"
    exit 1
fi

if [ "$BUILD_LIBOQS" = "1" ]; then
    echo "Building LibOQS..."
    if [ ! -d "libs/liboqs" ]; then
        echo "Error: LibOQS not found in libs/liboqs"
        echo "Please run: cd libs && git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git"
        exit 1
    fi
    
    cd libs/liboqs
    if [ ! -d "build" ]; then
        mkdir build
    fi
    cd build
    
    echo "Configuring LibOQS..."
    cmake -DCMAKE_INSTALL_PREFIX=.. \
          -DBUILD_SHARED_LIBS=ON \
          -DCMAKE_BUILD_TYPE=Release ..
    
    echo "Compiling LibOQS..."
    make -j$(nproc)
    
    echo "Installing LibOQS..."
    make install
    
    cd "$PROJECT_ROOT"
    echo "LibOQS built successfully"
    echo ""
fi

if [ "$BUILD_GMSSL" = "1" ]; then
    echo "Building GmSSL..."
    if [ ! -d "libs/GmSSL" ]; then
        echo "Error: GmSSL not found in libs/GmSSL"
        echo "Please run: cd libs && git clone --depth 1 https://github.com/guanzhi/GmSSL.git"
        exit 1
    fi
    
    cd libs/GmSSL
    if [ ! -d "build" ]; then
        mkdir build
    fi
    cd build
    
    echo "Configuring GmSSL..."
    cmake -DCMAKE_INSTALL_PREFIX=.. \
          -DCMAKE_BUILD_TYPE=Release ..
    
    echo "Compiling GmSSL..."
    make -j$(nproc)
    
    echo "Installing GmSSL..."
    make install
    
    cd "$PROJECT_ROOT"
    echo "GmSSL built successfully"
    echo ""
fi

echo "Building Unified Crypto Interface..."
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

echo "Configuring UCI..."
cmake -DUSE_LIBOQS=ON \
      -DUSE_GMSSL=ON \
      -DBUILD_EXAMPLES=ON \
      -DBUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Release ..

echo "Compiling UCI..."
make -j$(nproc)

echo ""
echo "======================================"
echo "Build completed successfully!"
echo "======================================"
echo ""
echo "Binaries are in: build/"
echo "Examples:"
echo "  - build/examples/uci_demo"
echo "  - build/examples/uci_list_algorithms"
echo "  - build/examples/uci_signature_demo"
echo "  - build/examples/uci_kem_demo"
echo ""
echo "To run tests:"
echo "  cd build && make test"
echo ""
echo "To install:"
echo "  cd build && sudo make install"
echo ""
