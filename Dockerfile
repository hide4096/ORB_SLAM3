FROM ubuntu:22.04

RUN apt update && apt upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
    git \
    cmake \
    libeigen3-dev \
    libboost-all-dev \
    libssl-dev \
    python3-dev \
    python3-pip \
    build-essential \
    libgl1-mesa-dev \
    libepoxy-dev \
    wget \
    unzip \
    libopencv-dev

RUN python3 -m pip install --upgrade pip setuptools wheel

# Pangolinのインストール
RUN cd /tmp && \
    git clone --recursive https://github.com/stevenlovegrove/Pangolin.git && \
    cd Pangolin && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

# ORB-SLAM3のインストール
RUN git clone  https://github.com/hide4096/ORB_SLAM3.git /ORB_SLAM3 && \
    cd /ORB_SLAM3 && \
    sed -i 's/++11/++14/g' CMakeLists.txt && \
    ./build.sh
