# Dockerfile for CUDA‑enabled llama.cpp image
# Uses NVIDIA PyTorch runtime base image (CUDA + cuDNN)

FROM nvcr.io/nvidia/pytorch:23.06-py3

# Build argument: absolute path to the directory that contains the binaries
ARG BINARY_SRC

# Copy executables and shared libraries into the container
COPY ${BINARY_SRC} /usr/local/bin/
COPY ${BINARY_SRC}/*.so* /usr/local/lib/

# Make executables runnable and register libraries
RUN chmod +x /usr/local/bin/* \
    && ldconfig

# Ensure the dynamic linker can find the libraries we just copied
ENV LD_LIBRARY_PATH=/usr/local/lib

# Expose the inference port
EXPOSE 8000

# just packs llama.cpp binaries
CMD ["/usr/local/bin/llama-diffuse-cli", "--help"]
