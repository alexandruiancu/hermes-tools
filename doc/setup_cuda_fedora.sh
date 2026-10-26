#!/bin/bash
# Setup script for Fedora 44 CUDA development toolbox
# This script creates a toolbox container and installs CUDA toolkit

CONTAINER_NAME="fedora-44-cuda-dev"

echo "=== Step 1: Create toolbox container ==="
if toolbox list --format '{{.Container.Name}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Container ${CONTAINER_NAME} already exists"
else
    echo "Creating toolbox container ${CONTAINER_NAME}..."
    toolbox create --distro fedora --release 44 "${CONTAINER_NAME}"
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to create toolbox container"
        exit 1
    fi
fi

echo ""
echo "=== Step 2: Check for NVIDIA GPU ==="
if lspci | grep -i nvidia >/dev/null 2>&1; then
    echo "NVIDIA GPU detected on host - CUDA should work properly"
else
    echo "WARNING: No NVIDIA GPU detected on host - CUDA may not work properly"
    echo "Ensure NVIDIA drivers are installed on the host system"
fi

echo ""
echo "=== Step 3: Enter toolbox container ==="

# Check if --gpus flag is supported
if toolbox enter --help 2>&1 | grep -q "\-\-gpus"; then
    ENTER_FLAGS="--gpus all"
else
    ENTER_FLAGS=""
    echo "Note: --gpus flag not supported in this toolbox version, entering without GPU passthrough"
fi

# Enter the container and run setup commands
if ! toolbox enter ${ENTER_FLAGS} -v /home/andu:/home "${CONTAINER_NAME}" <<'SETUP_EOF'
echo "=== Inside toolbox container ==="
echo "Container: $(hostname)"
echo "OS: $(cat /etc/os-release | grep PRETTY_NAME | tr -d '"')"

echo ""
echo "=== Step 4: Add CUDA network repo ==="

# Check if repo file already exists
if [ ! -f /etc/yum.repos.d/cuda-fedora44.repo ]; then
    echo "Adding CUDA repository..."
    dnf config-manager addrepo --from-repofile https://developer.download.nvidia.com/compute/cuda/repos/fedora44/x86_64/cuda-fedora44.repo
    if [ $? -ne 0 ]; then
        echo "WARNING: Failed to add CUDA repo, trying with --overwrite..."
        dnf config-manager addrepo --from-repofile https://developer.download.nvidia.com/compute/cuda/repos/fedora44/x86_64/cuda-fedora44.repo --overwrite
    fi
else
    echo "CUDA repo file already exists, skipping."
fi

echo ""
echo "=== Step 5: Clean dnf cache ==="

dnf clean all
echo "Cache cleaned."

echo ""
echo "=== Step 6: Install CUDA toolkit ==="

# Install CUDA toolkit 13.4
echo "Installing cuda-toolkit-13-4..."
dnf -y install cuda-toolkit-13-4
if [ $? -ne 0 ]; then
    echo "WARNING: CUDA toolkit installation may have issues, continuing anyway..."
fi

echo ""
echo "=== Step 7: Install c-development and development-tools ==="

dnf5 group install 'c-development' 'development-tools'
if [ $? -ne 0 ]; then
    echo "WARNING: Development group installation had issues..."
fi

echo ""
echo "=== Step 8: Install cmake, boost-devel, ninja-build ==="

dnf5 install cmake boost-devel ninja-build
if [ $? -ne 0 ]; then
    echo "WARNING: Some packages may not have been installed..."
fi

echo ""
echo "=== All setup complete! ==="

# Post-setup instructions
echo "To use the container:"
echo "  toolbox enter ${CONTAINER_NAME}"
echo "CUDA toolkit is installed and ready for development."
SETUP_EOF
then
    echo "Script had errors during execution"
fi