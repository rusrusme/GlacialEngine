#!/usr/bin/env bash
set -e

echo "=== Installing Linux Dependencies ==="

sudo apt update

# Build-Tools & Compiler
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    g++ \
    clang

# OpenGL, Mesa & EGL
sudo apt install -y \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libegl1-mesa-dev \
    mesa-utils \
    mesa-common-dev

# Vulkan & Shader-Compilation (Headers, Loader, Tools, SPIR-V & Shaderc für glacialc)
sudo apt install -y \
    libvulkan-dev \
    vulkan-tools \
    glslang-tools \
    libglslang-dev \
    spirv-tools \
    libspirv-tools-dev \
    libshaderc-dev

# X11 & SDL3 Extensions (XTest, ScrnSaver, Cursor, RandR, etc.)
sudo apt install -y \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxss-dev \
    libxtst-dev \
    libxext-dev \
    libxfixes-dev \
    libxkbcommon-dev

# Wayland Support
sudo apt install -y \
    libwayland-dev \
    wayland-protocols \
    extra-cmake-modules

# Audio (ALSA & PulseAudio für miniaudio)
sudo apt install -y \
    libasound2-dev \
    libpulse-dev

# Font Rendering (RmlUi / FreeType)
sudo apt install -y \
    libfreetype-dev

echo "=== All dependencies installed successfully ==="