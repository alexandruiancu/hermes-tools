#!/bin/bash
# Setup script for Arch Linux CUDA development environment
# This script installs CUDA toolkit and development packages

echo "=== Step 1: Check for root privileges ==="
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root or with sudo"
    exit 1
fi

echo ""
echo "=== Step 2: Check for NVIDIA drivers ==="
if pacman -Qs '^nvidia$' >/dev/null 2>&1; then
    echo "NVIDIA drivers already installed"
    NVIDIA_VERSION=$(pacman -Qs '^nvidia$' | grep -o 'Version : [0-9.]*' | head -1 | cut -d' ' -f3)
    echo "Driver version: ${NVIDIA_VERSION}"
else
    echo "WARNING: NVIDIA drivers not found in system"
    echo "Install with: sudo pacman -S nvidia"
    echo "NVIDIA drivers should be installed BEFORE CUDA toolkit for proper functionality"
    echo "Would you like to install them now? (y/n)"
    read -r install_drivers
    if [[ "$install_drivers" =~ ^[Yy]$ ]]; then
        sudo pacman -S --needed nvidia
    fi
fi

echo ""
echo "=== Step 3: Update package databases ==="
pacman -Sy

echo ""
echo "=== Step 4: Install CUDA toolkit ==="
pacman -S --needed cuda

echo ""
echo "=== Step 5: Verify CUDA installation ==="
CUDA_VERSION=$(pacman -Qs '^cuda$' | grep -o 'Version : [0-9.]*' | head -1 | cut -d' ' -f3)
echo "CUDA toolkit version: ${CUDA_VERSION}"
nvcc --version
if [ $? -ne 0 ]; then
    echo "WARNING: nvcc not found, CUDA may not be properly installed"
fi

echo ""
echo "=== Step 6: Install development tools ==="
pacman -S --needed gcc cmake boost ninja make git curl wget

echo ""
echo "=== Step 7: Install CUDA samples ==="
pacman -S --needed cuda-samples

echo ""
echo "=== Step 8: Set up environment variables ==="
if ! grep -q "CUDA_HOME" /etc/profile.d/cuda.sh 2>/dev/null; then
    echo "export CUDA_HOME=/usr/local/cuda" > /etc/profile.d/cuda.sh
    echo "export PATH=\$CUDA_HOME/bin:\$PATH" >> /etc/profile.d/cuda.sh
    echo "export LD_LIBRARY_PATH=\$CUDA_HOME/lib64:\$LD_LIBRARY_PATH" >> /etc/profile.d/cuda.sh
    chmod +x /etc/profile.d/cuda.sh
    echo "Environment variables set up in /etc/profile.d/cuda.sh"
else
    echo "CUDA environment variables already configured"
fi

echo ""
echo "=== All setup complete! ==="
echo "CUDA toolkit installed successfully (version: ${CUDA_VERSION})"
echo "Sample programs are available in /opt/cuda/samples/"
echo "Run 'source /etc/profile.d/cuda.sh' to set up environment"