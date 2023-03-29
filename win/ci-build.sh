#!/bin/bash
#
# References:
#
#	* https://www.msys2.org/docs/ci/
#
#
echo "Running ${0}"

TARGET_ARCH="x86_64"

LOGFILE=build.log
rm -f ${LOGFILE}

die ( ) {
	[ -s $LOGFILE ] && tail $LOGFILE
	[ "$1" ] && echo "$*"
	exit -1
}

myDIR=$(dirname $(dirname $(readlink -f ${0})))
echo "myDIR=${myDIR}"

cd ${myDIR}
rm -fr ${myDIR}/.build

#
# Unpack lib3270
#
echo "Unpacking lib3270"
tar -C / -Jxf mingw-lib3270.${TARGET_ARCH}.tar.xz > $LOGFILE 2>&1 || die "lib3270 unpack failure"

#
# Build libv3270
#
echo "Building libv3270"
cd ${myDIR}
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"

make DESTDIR=.bin/package install || die "Install failure"
tar --create --xz --file=mingw-libv3270.${TARGET_ARCH}.tar.xz --directory=.bin/package --verbose . || die "Tar failure"


echo "Build complete"

