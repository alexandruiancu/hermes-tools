#!/bin/bash
# Setup script for Ubuntu CUDA development environment
# This script creates a Multipass container and installs CUDA toolkit

CONTAINER_NAME="ubuntu-cuda-dev"
UBUNTU_RELEASE="22.04"

echo "=== Step 1: Check for Multipass ==="
if command -v multipass &>/dev/null; then
    echo "Multipass is installed"
else
    echo "Installing Multipass..."
    sudo snap install multipass --classic
fi

echo ""
echo "=== Step 2: Create Multipass container ==="
if multipass list --name | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container ${CONTAINER_NAME} already exists"
else
    echo "Creating Multipass container ${CONTAINER_NAME}..."
    multipass launch "${UBUNTU_RELEASE}" --name "${CONTAINER_NAME}" --cpus 4 --memory 8G --disk 50G
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to create Multipass container"
        exit 1
    fi
fi

echo ""
echo "=== Step 3: Start container and set up CUDA ==="
multipass info "${CONTAINER_NAME}" | head -5

echo ""
echo "=== Step 4: Add CUDA repository ==="
multipass exec "${CONTAINER_NAME}" -- bash -c "
    # Update package lists
    sudo apt-get update

    # Add CUDA repository GPG keyring (modern approach - apt-key is deprecated)
    sudo mkdir -p /etc/apt/keyrings
    wget -qO- https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/3bf863cc.pub | \
        sudo gpg --dearmor -o /etc/apt/keyrings/nvidia-cuda-keyring.gpg

    # Add CUDA repository
    echo 'deb [signed-by=/etc/apt/keyrings/nvidia-cuda-keyring.gpg] https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/ /' | \
        sudo tee /etc/apt/sources.list.d/cuda.list

    # Accept CUDA repository pin
    wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-ubuntu2204.pin
    sudo mv cuda-ubuntu2204.pin /etc/apt/preferences.d/cuda-repository-pin-600

    sudo apt-get update
"

echo ""
echo "=== Step 5: Install CUDA toolkit ==="
multipass exec "${CONTAINER_NAME}" -- bash -c "
    sudo apt-get install -y cuda-toolkit-12-4
    if [ $? -ne 0 ]; then
        echo 'WARNING: CUDA toolkit installation may have issues'
    fi
"

echo ""
echo "=== Step 6: Install development tools ==="
multipass exec "${CONTAINER_NAME}" -- bash -c "
    sudo apt-get install -y build-essential cmake git curl wget
    if [ $? -ne 0 ]; then
        echo 'WARNING: Some development tools may not have been installed'
    fi
"

echo ""
echo "=== Step 7: Install CUDA samples ==="
multipass exec "${CONTAINER_NAME}" -- bash -c "
    sudo apt-get install -y cuda-samples-12-4
    if [ $? -ne 0 ]; then
        echo 'WARNING: CUDA samples installation may have issues'
    fi
"

echo ""
echo "=== All setup complete! ==="
echo "Container: ${CONTAINER_NAME}"
echo "Use 'multipass shell ${CONTAINER_NAME}' to enter the container"
echo "CUDA paths are typically in /usr/local/cuda-12.4/"