# Stage 1

FROM ubuntu:latest as stage1

# Setup timezone (needed by some packages on installation).
RUN ln -snf /usr/share/zoneinfo/Europe/Zurich /etc/localtime && \
    echo Europe/Zurich > /etc/timezone && \
    echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections && \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    wget bc bison bridge-utils ca-certificates cpio device-tree-compiler \
    elfutils file flex gcc git g++ lib32z1-dev libc6:i386 libgtk2.0-dev \
    libssl-dev libstdc++6:i386 make wget openssh-server patch \
    pkg-config xz-utils unzip bzip2 rsync gnupg gnupg1 gnupg2 curl tzdata ca-certificates

RUN mkdir -p /opt/toolchains && cd /opt/toolchains &&   \
    #Download and extract arm-none-linux-gnueabihf toolchain
    wget -q https://snapshots.linaro.org/gnu-toolchain/11.3-2022.06-1/arm-linux-gnueabihf/gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf.tar.xz

RUN cd /opt/toolchains && \
    tar -xf gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf.tar.xz &&  \
    rm gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf.tar.xz && \
    echo export PATH=${PATH}:/opt/toolchains/gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf/bin >> /root/.bashrc

# Install aarch64 toolchain
RUN wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz
RUN tar -xvf gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz
RUN rm gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz

ENV PATH="$PATH:/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin"

COPY . /root/pos

WORKDIR /root/pos
