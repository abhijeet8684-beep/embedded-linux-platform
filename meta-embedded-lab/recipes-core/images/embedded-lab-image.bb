SUMMARY = "Embedded lab reference image"
DESCRIPTION = "Reference Yocto image for the EDU virtual hardware platform"
LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += "ssh-server-openssh package-management"

IMAGE_INSTALL += "packagegroup-core-boot \
                  edu-device \
                  edu-cli"

IMAGE_ROOTFS_SIZE = "250000"
