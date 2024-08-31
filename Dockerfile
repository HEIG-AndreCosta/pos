# Stage 1

FROM alpine:latest as stage1
ARG SO3_BRANCH=main

 AArch32
RUN apk update; \
    apk add make cmake gcc-arm-none-eabi libc-dev \
    bison flex bash patch mount dtc \
    dosfstools u-boot-tools net-tools \
    bridge-utils iptables dnsmasq libressl-dev \
    util-linux qemu-system-arm e2fsprogs

# AArch64
#RUN apk update; \
#    apk add make cmake gcc-aarch64-none-elf libc-dev \
#    bison flex bash patch mount dtc \
#    dosfstools u-boot-tools net-tools \
#    bridge-utils iptables dnsmasq libressl-dev \
#    util-linux qemu-system-aarch64 e2fsprogs


RUN cd /; \
    echo "SO3 zip file URL: https://github.com/smartobjectoriented/so3/archive/refs/heads/${SO3_BRANCH}.zip";\
    wget "https://buildroot.org/downloads/buildroot-2024.02.5.tar.gz"; \
    tar xf build*;
    

