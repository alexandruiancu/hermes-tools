# setup_cuda_arch.sh

A Bash script that sets up a Arch Linux system with the CUDA development toolkit.

## What it does

1. **Verifies root privileges** - requires root access for system-wide package installation
2. **Checks for NVIDIA drivers** - verifies if NVIDIA drivers are installed; installs if missing
3. **Updates package databases** - syncs `pacman` repositories
4. **Installs CUDA toolkit** - `cuda` package from official repositories
5. **Verifies CUDA installation** - reports CUDA version and checks `nvcc` compiler
6. **Installs development tools** - `gcc`, `cmake`, `boost`, `ninja`, `make`, `git`, `curl`, `wget`
7. **Installs CUDA samples** - `cuda-samples` package
8. **Sets up environment variables** - configures `CUDA_HOME`, `PATH`, and `LD_LIBRARY_PATH`

## Prerequisites

- Arch Linux operating system
- Root privileges (or run with `sudo`)
- NVIDIA GPU with appropriate drivers (automatically checked/installed if missing)
- Internet access for package downloads

## Usage

```bash
sudo bash /home/andu/src/hermes-tools/setup_cuda_arch.sh
```

## Notes

- The script uses `pacman`, Arch Linux's native package manager
- **NVIDIA driver check**: Script automatically detects if NVIDIA drivers are installed and installs them if missing before proceeding with CUDA installation. This ensures a smooth setup process without manual driver installation steps.
- CUDA version installed corresponds to the latest available in the official repositories
- Environment variables are configured system-wide in `/etc/profile.d/cuda.sh`
- Sample CUDA programs are installed to `/opt/cuda/samples/`
- After running the script, remember to run `source /etc/profile.d/cuda.sh` or restart your terminal
- GPU drivers are now automatically handled by the script, but you should verify installation

## Post-installation

To verify the installation works:

```bash
source /etc/profile.d/cuda.sh
nvcc --version
```

Or compile and run a sample:

```bash
cd /opt/cuda/samples
make
./hellocl
```