# setup_cuda_toolbox.sh

A Bash script that sets up a Fedora 44 toolbox container with the CUDA development toolkit.

## What it does

1. **Creates a toolbox container** named `fedora-44-cuda-dev` if it doesn't already exist
2. **Checks for NVIDIA GPU** on the host system - warns if no NVIDIA GPU detected
3. **Enters the container** with GPU support (auto-detected; `--gpus all` if supported)
4. **Adds the CUDA repository** from NVIDIA (skips if already present)
5. **Cleans the dnf cache**
6. **Installs CUDA toolkit 13.4**
7. **Installs development groups**: `c-development` and `development-tools`
8. **Installs build tools**: `cmake`, `boost-devel`, `ninja-build`

## Prerequisites

- Fedora 44 host system
- `toolbox` installed and running
- NVIDIA GPU with appropriate drivers on the host system
- Internet access for package downloads

## Usage

```bash
bash /home/andu/src/hermes-tools/setup_cuda_toolbox.sh
```

## Notes

- The script uses `toolbox` container technology to isolate the CUDA development environment
- **NVIDIA GPU detection**: Script checks for NVIDIA GPU on host before proceeding; warns if not found
- **GPU access**: If `--gpus` is supported by your toolbox version, it's automatically passed to the container. Otherwise, container enters without GPU passthrough (you can install drivers inside later if needed)
- CUDA repository is added from NVIDIA's developer site; existing repo is skipped
- Installation may require accepting DNF prompts interactively
- After setup, use `toolbox enter fedora-44-cuda-dev` to access your CUDA development environment