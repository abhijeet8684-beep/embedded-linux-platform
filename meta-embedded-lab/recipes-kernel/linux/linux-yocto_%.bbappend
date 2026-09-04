FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://edu-device.dts"

KERNEL_DEVICETREE:append = " edu-device.dtb"

do_configure:append() {
    if [ -f "${WORKDIR}/edu-device.dts" ]; then
        install -m 0644 "${WORKDIR}/edu-device.dts" "${B}/arch/${ARCH}/boot/dts/edu-device.dts"
    fi
}
