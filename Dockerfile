# Dockerfile: Recipe for building a Docker image
# Each line is a layer that gets cached for faster rebuilds

# Base image - minimal Ubuntu 22.04
# FROM = start from this existing image
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set working directory inside container
WORKDIR /workspace

# Update package lists and install essential tools
# RUN = execute command during image build
# && chains commands (if first succeeds, run second)
# \ continues command on next line for readability
RUN apt-get update && apt-get install -y \
    # Build essentials: gcc, make, binutils, etc.
    build-essential \
    # CMake + Ninja for project build (generates compile_commands.json for editor)
    cmake \
    ninja-build \
    # clangd for C/C++ navigation when using Dev Containers
    clangd \
    # ARM cross-compiler toolchain (for STM32/ARM microcontrollers)
    gcc-arm-none-eabi \
    # OpenOCD - on-chip debugger for flashing/debugging
    openocd \
    # Git for version control
    git \
    # Python3 and pip for embedded tools
    python3 \
    python3-pip \
    # Serial communication tools
    picocom \
    screen \
    # Debugging tools
    gdb \
    gdb-multiarch \
    # Make sure we have basic utilities
    vim \
    nano \
    curl \
    wget \
    # Clean up apt cache to reduce image size
    && rm -rf /var/lib/apt/lists/*

# Install Starship prompt (cross-shell, git branch, colors, etc.)
RUN curl -sS https://starship.rs/install.sh | sh -s -- -y -b /usr/local/bin

# Install Python packages commonly used in embedded development
RUN pip3 install --no-cache-dir \
    pyserial \
    intelhex \
    pyelftools

# Create a non-root user for development (best practice)
# Running as root in containers is a security risk
RUN useradd -m -s /bin/bash developer && \
    usermod -aG dialout developer && \
    chown -R developer:developer /workspace

# Switch to non-root user
USER developer

# Default command when container starts
# This keeps container running so you can exec into it
CMD ["/bin/bash"]
