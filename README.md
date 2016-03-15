# OpenCL-test-apps
Applications for OpenCL testing on Toradex Apalis iMX6Q

For compiling the applications please follow the steps:

1 - Find in your system the cl.h header file. use the following command:
    find / -iname cl.h 2>/dev/null
    
    You should find something like this:
    ~/oe-core/build/out-glibc/sysroots/apalis-imx6/usr/include/CL/cl.h

2 - Edit the Makefile setting TOOLCHAIN and CROSS_COMPILER according to your system

3 - Set the environment variables:
    export ARCH=arm
    export ROOTFS_DIR=/usr/local/toradex-linux-v2.5/oe-core/build/out-glibc/sysroots/apalis-imx6
    
    ROOTFS_DIR should be where the cl.h header resides.

4 - Run make
    
    
