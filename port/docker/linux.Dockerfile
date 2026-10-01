# The Linux build environment (`uv run port build linux-x64|linux-arm64`): the
# compiler, CMake and the libraries SDL2 is built against. Ubuntu 22.04 because
# the program links glibc dynamically, so it runs on that glibc (2.35) and
# newer, which covers current Ubuntu, Debian, Fedora and SteamOS. SDL2 itself is
# built from source, statically (port/CMakeLists.txt), and opens the X11,
# Wayland, PulseAudio and ALSA libraries at run time, so the program needs none
# of them installed to start.

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates clang lld pkg-config python3-pip \
        libasound2-dev libpulse-dev libdbus-1-dev libudev-dev libibus-1.0-dev \
        libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev \
        libxss-dev libxkbcommon-dev libwayland-dev wayland-protocols \
        libdrm-dev libgbm-dev libegl1-mesa-dev libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# The port needs CMake 3.24 or later; Ubuntu 22.04 has 3.22.
RUN pip3 install --no-cache-dir 'cmake>=3.24' ninja

ENV CC=clang CXX=clang++ HOME=/tmp
