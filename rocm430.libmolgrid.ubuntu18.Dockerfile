# Copyright (c) 2021 Advanced Micro Devices, Inc. All Rights Reserved.
#
# ROCm-4.3.0 Dockerfile for Ubuntu18 for libmolgrid
#

FROM ubuntu:18.04

RUN sed -i -e "s/\/archive.ubuntu/\/us.archive.ubuntu/" /etc/apt/sources.list && \
    apt-get clean && \
    apt-get -y update --fix-missing --allow-insecure-repositories && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    aria2 \
    autoconf \
    bc \
    bison \
    bzip2 \
    check \
    cifs-utils \
    cmake \
    curl \
    dkms \
    dos2unix \
    doxygen \
    flex \
    g++-multilib \
    gcc-multilib \
    git \
    locales \
    libatlas-base-dev \
    libbabeltrace1 \
    libboost-all-dev \
    libboost-program-options-dev \
    libelf-dev \
    libelf1 \
    libfftw3-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libhdf5-serial-dev \
    libleveldb-dev \
    liblmdb-dev \
    libnuma-dev \
    libopenblas-base \
    libopenblas-dev \
    libopencv-dev \
    libpci3 \
    libpython3.8 \
    libfile-which-perl \
    libprotobuf-dev \
    libsnappy-dev \
    libssl-dev \
    libunwind-dev \
    ocl-icd-dev \
    ocl-icd-opencl-dev \
    pkg-config \
    protobuf-compiler \
    python-numpy \
    python-pip \
    python-pip-whl \
    python-scipy \
    python-yaml \
    python3-dev \
    python3-pip \
    ssh \
    swig \
    sudo \
    unzip \
    vim \
    xsltproc && \
    pip3 install Cython && \
    pip3 install numpy && \
    pip3 install optionloop && \
    pip3 install protobuf && \
    pip3 install networkx && \
    pip install Cython && \
    pip install numpy && \
    pip install optionloop && \
    pip install setuptools && \
    pip install CppHeaderParser argparse && \
    ldconfig && \
    cd $HOME && \
    mkdir -p downloads && \
    cd downloads && \
    wget -O rocminstall.py --no-check-certificate https://raw.githubusercontent.com/srinivamd/rocminstaller/master/rocminstall.py && \
    python3 ./rocminstall.py --nokernel --rev 4.3 && \
    cd $HOME && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* downloads

# Set up paths
ENV ROCM_VERSION=4.3.0
ENV ROCM_PATH="/opt/rocm-${ROCM_VERSION}"
ENV PATH="${ROCM_PATH}/bin:${ROCM_PATH}/opencl/bin:${PATH}"

#
RUN /bin/sh -c 'ln -sf ${ROCM_PATH} /opt/rocm'

#
RUN locale-gen en_US.UTF-8

ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

#Install Openbabel and Update CMake
RUN cd /home/ && \
    mkdir -p repo && \
    cd /home/repo/ && \ 
    apt clean && \
    apt update && \
    apt install -y  git build-essential libboost-all-dev python3-pip rapidjson-dev && \
    pip3 install numpy pytest pyquaternion && \
    cd /home/repo/ && \
    wget https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5.tar.gz && \
    tar -zxvf cmake-3.20.5.tar.gz && \
    cd /home/repo/cmake-3.20.5/ && \
    ./bootstrap && \
    make && \
    make install && \
    hash -r && \
    cd /home/repo/ && \
    git clone --recursive https://github.com/openbabel/openbabel && \
    cd /home/repo/openbabel && \
    git checkout -b op3.1.1 tags/openbabel-3-1-1 && \
    mkdir -p build && \
    cd /home/repo/openbabel/build/ && \
    cmake .. && \
    make && \
    make install && \
    cd /home/repo/ && \
    apt install -y libeigen3-dev libboost-all-dev && \
    cd /home/repo/ && \
    git clone --recursive https://github.com/sidamd/libmolgrid && \
    cd /home/repo/libmolgrid/ && \
    git checkout rocm_port_4.3_libmolgrid && \
    mkdir -p build/ && \
    cd /home/repo/libmolgrid/build && \
    cmake -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_CXX_COMPILER=/opt/rocm-4.3.0/bin/hipcc -DUSE_ROCM=/opt/rocm-4.3.0 .. && \
    sed -i -e "s/static T seq/__host__ static T seq/"  /opt/rocm-4.3.0/include/thrust/system/hip/detail/reduce.h && \
    sed -i -e "s/static void seq/__host__ static void seq/" /opt/rocm-4.3.0/include/thrust/system/hip/detail/internal/copy_cross_system.h && \
    make

     



# Default to a login shell
CMD ["bash", "-l"]
