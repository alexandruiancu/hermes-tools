# setup_cuda_ubuntu.sh

A Bash script that sets up an Ubuntu 22.04 Multipass container with the CUDA development toolkit.

## What it does

1. **Checks for Multipass** - installs it if not present
2. **Creates a Multipass container** named `ubuntu-cuda-dev` running Ubuntu 22.04 if it doesn't already exist
3. **Adds the CUDA repository** from NVIDIA using modern GPG keyring (replaces deprecated `apt-key`)
4. **Installs CUDA toolkit 12.4**
5. **Installs development tools**: `build-essential`, `cmake`, `git`, `curl`, `wget`
6. **Installs CUDA samples**

## Prerequisites

- Ubuntu 22.04 host system (or compatible)
- Multipass installed (auto-installed if missing)
- Internet access for package downloads
- NVIDIA GPU with appropriate drivers on the host

## Usage

```bash
bash /home/andu/src/hermes-tools/setup_cuda_ubuntu.sh
```

## Notes

- The script uses **Multipass** container technology to isolate the CUDA development environment
- CUDA 12.4 is installed (latest long-term support release at time of writing)
- **Modern GPG keyring approach**: The script adds the NVIDIA CUDA GPG key to `/etc/apt/keyrings/nvidia-cuda-keyring.gpg` instead of using the deprecated `apt-key` command
- Repository is configured with `deb [signed-by=/etc/apt/keyrings/nvidia-cuda-keyring.gpg]` for proper signature verification
- Container persists after script completion - use `multipass shell ubuntu-cuda-dev` to enter
- CUDA libraries are typically available at `/usr/local/cuda-12.4/`
- GPU access requires proper NVIDIA driver configuration on the host system
- This approach is compatible with Ubuntu 22.04 and newer versions that deprecate `apt-key`