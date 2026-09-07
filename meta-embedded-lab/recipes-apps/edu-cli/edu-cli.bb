SUMMARY = "EDU CLI diagnostic tool"
DESCRIPTION = "Command-line probe and test utility for the EDU virtual device"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=46e04580be7dfe845cc69e708e7ec155"

SRC_URI = "file://edu-cli.c \
           file://Makefile \
           file://LICENSE"

S = "${WORKDIR}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 edu-cli ${D}${bindir}/edu-cli
}

FILES:${PN} = "${bindir}/edu-cli"
