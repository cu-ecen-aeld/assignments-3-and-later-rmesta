#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "${OUTDIR}"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    #
    # Rick Mesta
    # 07/17/2024
    #
    # University of Colorado at Boulder
    # ECEN 5713: Advanced Embedded Linux Development
    # Assignment 3 Part 2
    #

    # Deep clean
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    # Use default config
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    # Build kernel for ${ARCH}
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    # Skipping modules for Assignment 3
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    # Build Dev Tree
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp -p ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/.


echo "Creating the staging directory for the root filesystem"
cd "${OUTDIR}"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

#
# Rick Mesta
# 07/17/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 3 Part 2
#

# Create necessary base directories

mkdir "${OUTDIR}/rootfs" && cd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "${OUTDIR}"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}

    # Configure busybox

    echo -e "\033[0;92mConfiguring Busybox ${BUSYBOX_VERSION}\033[0m"
    make distclean
    make defconfig

    # Build busybox

    echo -e "\033[0;92mBuilding Busybox ${BUSYBOX_VERSION}\033[0m"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}

else
    cd busybox
fi

# Install busybox

echo -e "\033[0;92mInstalling Busybox in rootfs\033[0m"
make CONFIG_PREFIX="${OUTDIR}/rootfs" ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

# Add library dependencies to rootfs

echo "Library dependencies"

cd "${OUTDIR}/rootfs"
ALIB=$(${CROSS_COMPILE}readelf -a bin/busybox   |\
    egrep "program interpreter"                 |\
    awk '{print $NF}'                           |\
    sed -e 's/\(.*\)]/\1/')

echo "Copying ${SYSROOT}${ALIB} to ${OUTDIR}/rootfs/lib64"
sudo cp -p "${SYSROOT}${ALIB}" "${OUTDIR}/rootfs/lib64/."
sudo cp -p "${SYSROOT}${ALIB}" "${OUTDIR}/rootfs/lib/."

LIBLST=$(${CROSS_COMPILE}readelf -a bin/busybox |\
    egrep "Shared library"                      |\
    awk '{print $NF}')

for i in ${LIBLST}; do
    SL=$(echo ${i} | sed -e 's/\[\(.*\)\]/\1/')
    echo "Copying ${SYSROOT}/lib64/${SL} to ${OUTDIR}/rootfs/lib64"
    sudo cp -p "${SYSROOT}/lib64/${SL}" "${OUTDIR}/rootfs/lib64/."
done


# Make device nodes

cd "${OUTDIR}/rootfs"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# Clean and build the writer utility

cd ${FINDER_APP_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}


# Copy the finder related scripts and executables to the
# /home directory on the target rootfs

MANIFEST="autorun-qemu.sh finder.sh finder-test.sh writer"
for s in ${MANIFEST}; do
    if [ ${s} == "finder-test.sh" ]; then
        sed -e 's|\.\.\/||' ${FINDER_APP_DIR}/${s} > ${OUTDIR}/${s}
        sudo mv -f ${OUTDIR}/${s} ${OUTDIR}/rootfs/home/${s}
        sudo chmod 755 ${OUTDIR}/rootfs/home/${s}
    else
        sudo cp -p "${FINDER_APP_DIR}/${s}" "${OUTDIR}/rootfs/home"
    fi
done

CFG_DIR=${OUTDIR}/rootfs/home/conf
sudo mkdir -p ${CFG_DIR}

CCONFIGS="conf/username.txt conf/assignment.txt"
for c in ${CCONFIGS}; do
    sudo cp -p "${FINDER_APP_DIR}/${c}" "${CFG_DIR}"
done

# Chown the root directory

sudo chown -R root:root ${OUTDIR}/rootfs
sudo chmod u+s ${OUTDIR}/rootfs/bin/busybox

# Create initramfs.cpio.gz

cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio

[ $? -eq 0 ] && echo -e "\033[1;92mSuccess !!!\033[0m" && exit 0

echo -e "\033[1;91mSomething went wrong... \033[0m" && exit 1
