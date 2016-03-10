#	para encontrar biblioteca CL use o seguinte comando
#	find / -iname cl.h 2>/dev/null
#	/usr/local/toradex-linux-v2.5/oe-core/build/out-glibc/sysroots/apalis-imx6/usr/include/CL/cl.h
#	antes do comando make, execute:
#	export ARCH=arm
#	export ROOTFS_DIR=/usr/local/toradex-linux-v2.5/oe-core/build/out-glibc/sysroots/apalis-imx6

#	for colibri imx6
#	export ROOTFS_DIR=/usr/local/toradex-linux-v2.5/oe-core/build/out-glibc/sysroots/colibri-imx6

APPNAME = cl_sample_timer
TOOLCHAIN = /home/prjs/toolchain/gcc-linaro

CROSS_COMPILER = $(TOOLCHAIN)/bin/arm-linux-gnueabihf-
CC= $(CROSS_COMPILER)gcc
DEL_FILE = rm -rf
CP_FILE = cp -rf
TARGET_PATH_LIB = $(ROOTFS_DIR)/usr/lib
TARGET_PATH_INCLUDE = $(ROOTFS_DIR)/usr/include
CFLAGS = -DLINUX -DUSE_SOC_MX6 -Wall -std=c99 -O2 -fsigned-char -march=armv7-a -mfpu=neon -DEGL_API_FB -DGPU_TYPE_VIV -DGL_GLEXT_PROTOTYPES -DENABLE_GPU_RENDER_20 -I../include -I$(TARGET_PATH_INCLUDE)
LFLAGS = -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB) -lm -lglib-2.0 -lOpenCL -lCLC -ldl -lpthread
OBJECTS = $(APPNAME).o
first: all
all: $(APPNAME)
$(APPNAME): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(APPNAME) $(OBJECTS)
$(APPNAME).o: $(APPNAME).c
	$(CC) $(CFLAGS) -c -o $(APPNAME).o $(APPNAME).c
clean:
	$(DEL_FILE) $(APPNAME)
