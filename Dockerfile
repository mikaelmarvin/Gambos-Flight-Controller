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
    # lsusb — list USB devices (ST-Link, serial adapters) when debugging connection
    usbutils \
    # Clean up apt cache to reduce image size
    && rm -rf /var/lib/apt/lists/*

# Ubuntu's gcc-arm-none-eabi does not install arm-none-eabi-gdb; gdb-multiarch debugs Cortex-M.
# Provide the usual name so Cortex-Debug and CLI workflows match ARM docs.
RUN ln -sf /usr/bin/gdb-multiarch /usr/bin/arm-none-eabi-gdb

# Install Starship prompt (cross-shell, git branch, colors, etc.)
RUN curl -sS https://starship.rs/install.sh | sh -s -- -y -b /usr/local/bin

# Bake Starship into /etc/skel before useradd so /home/developer gets it from the image.
# When dev-home volume is first populated from the image, .bashrc includes this.
RUN echo '' >> /etc/skel/.bashrc \
    && echo '# Starship prompt (gambos image)' >> /etc/skel/.bashrc \
    && echo '[ -n "$BASH_VERSION" ] && command -v starship >/dev/null 2>&1 && eval "$(starship init bash)"' >> /etc/skel/.bashrc

# Install Python packages commonly used in embedded development
RUN pip3 install --no-cache-dir \
    pyserial \
    intelhex \
    pyelftools

# SEGGER J-Link Software Pack (JLinkExe): reliable flashing from the container over USB.
# Ubuntu's OpenOCD + libjaylink often fails to open J-Link ("No J-Link device found") even when
# /dev/bus/usb sees the probe; the official SEGGER stack works with the same compose USB passthrough.
# License: download POST accepts SEGGER's click-through license (same as manual install).
# J-Link postinst runs udevadm to reload rules; Docker has no udevd, so real udevadm fails.
# dpkg maintainer scripts often omit /usr/local/bin from PATH; /usr/bin/udevadm is always found.
RUN set -eux; \
    ARCH="$(dpkg --print-architecture)"; \
    case "$ARCH" in \
        amd64) JLINK_DEB_URL="https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb" ;; \
        arm64) JLINK_DEB_URL="https://www.segger.com/downloads/jlink/JLink_Linux_arm64.deb" ;; \
        *) echo "Skipping SEGGER J-Link install: no .deb for architecture ${ARCH}"; JLINK_DEB_URL="" ;; \
    esac; \
    if [ -n "${JLINK_DEB_URL}" ]; then \
        apt-get update; \
        printf '%s\n' '#!/bin/sh' 'exit 0' > /usr/bin/udevadm && chmod 755 /usr/bin/udevadm; \
        wget --quiet --post-data "accept_license_agreement=accepted" -O /tmp/JLink.deb "${JLINK_DEB_URL}"; \
        apt-get install -y --no-install-recommends /tmp/JLink.deb; \
        rm -f /tmp/JLink.deb /usr/bin/udevadm; \
        rm -rf /var/lib/apt/lists/*; \
    fi

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
