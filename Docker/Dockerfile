FROM nvidia/opengl:1.0-glvnd-runtime-ubuntu20.04

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y

ARG USER
ENV USERNAME=$USER
RUN useradd -m -G sudo ${USERNAME} && \
    echo ${USERNAME}:U6aMy0wojraho | chpasswd -e

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    software-properties-common \
    apt-transport-https \
    wget \
    sudo \
    net-tools \
    libcanberra-gtk-module \
    apt-utils \
    language-pack-en \
    libgl1-mesa-dri \
    libnss3 \
    python3 \
    xdg-utils \
    libcurl4 \
    libxss1 \
    libxkbcommon-x11-0 \
    gdb \
    gcc \
    g++ \
    make \
    libglu1-mesa-dev \
    cmake \
    libgl-dev \
    libxml2-dev \
    zlib1g-dev \
    geoclue-2.0 \
    libgl-dev \
    python3-pip \
    libopencv-dev \
    pkg-config && \
    rm -rf /var/lib/apt/lists/*

USER ${USERNAME}
RUN python3 -m pip install --upgrade pip

USER root
WORKDIR /home/${USERNAME}
COPY ./bashrc /home/${USERNAME}/bashrc
COPY ./docker-entrypoint.sh /home/${USERNAME}/docker-entrypoint.sh
RUN chmod +x /home/${USERNAME}/docker-entrypoint.sh

## Required for NVIDIA compatibility
RUN mkdir -p /etc/OpenCL/vendors && echo "libnvidia-opencl.so.1" > /etc/OpenCL/vendors/nvidia.icd
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=compute,utility

ENTRYPOINT /home/${USERNAME}/docker-entrypoint.sh
USER root
