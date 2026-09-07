FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://edu-device.dts"

KERNEL_DEVICETREE:append = " edu-device.dtb"

do_configure:append() {
    if [ "${ARCH}" = "arm64" ] && [ -f "${WORKDIR}/edu-device.dts" ]; then
        install -d "${B}/arch/arm64/boot/dts"
        install -m 0644 "${WORKDIR}/edu-device.dts" "${B}/arch/arm64/boot/dts/edu-device.dts"
    fi
}
