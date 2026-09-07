SUMMARY = "EDU virtual device kernel module"
DESCRIPTION = "Platform and character-driver support for the EDU virtual device"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://Makefile \
           file://edu_device.h \
           file://edu_main.c \
           file://edu_chrdev.c \
           file://edu_fifo.c \
           file://edu_irq.c \
           file://edu_hw.c \
           file://edu_sysfs.c \
           file://edu_debugfs.c \
           file://edu_recovery.c \
           file://edu_pm.c \
           file://edu_ioctl.h"

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "edu_device"

RPROVIDES:${PN} += "kernel-module-edu-device"
