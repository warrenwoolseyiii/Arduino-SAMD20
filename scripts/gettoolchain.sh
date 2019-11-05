#!/bin/bash

TC_URL_LINUX='https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2?revision=bc2c96c0-14b5-4bb4-9f18-bceb4050fee7?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,7-2018-q2-update'
TC_NAME_LINUX='gcc-arm-none-eabi-7-2018-q2-update-linux'
TC_BZ2_LINUX='gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2'

TC_URL_OSX='https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-mac.tar.bz2?revision=982ef8a4-1815-4651-9c44-6144c9d8b34b?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Mac%20OS%20X,7-2018-q2-update'
TC_NAME_OSX='gcc-arm-none-eabi-7-2018-q2-update-mac'
TC_BZ2_OSX='gcc-arm-none-eabi-7-2018-q2-update-mac.tar.bz2'

case "$1" in
	"linux") TC_URL=$TC_URL_LINUX; TC_NAME=$TC_NAME_LINUX; TC_BZ2=$TC_BZ2_LINUX;;
	"osx") TC_URL=$TC_URL_OSX; TC_NAME=$TC_NAME_OSX; TC_BZ2=$TC_BZ2_OSX;;
	*) echo "Error: invalid arg \"$1\". Must be 'osx' or 'linux'." && exit 1;;
esac

THISDIR=$(dirname `realpath $0`);
DLDIR=$(realpath $THISDIR/../dl);
TCDIR=$(realpath $THISDIR/../toolchain);

source $THISDIR/utils.sh;
! [[ $? -eq 0 ]] && echo "FATAL: Unable to source $THISDIR/utils.sh" && exit 1;

if [[ -d $TCDIR/arm-none-eabi ]]; then
	do_warn "Skipping as $TCDIR/arm-none-eabi already exists";
	exit 0;
fi

# Download bz2 file if DNE
mkdir -p dl;
if ! [[ -f dl/$TC_BZ2 ]]; then
	do_info "Downloading $TC_BZ2 from $TC_URL";
	wget "$TC_URL" -O dl/$TC_BZ2;
	! [[ $? -eq 0 ]] && do_error "failed to download $TC_BZ2" && exit 1;
fi

rm -rf $TC_NAME;
tar xjf dl/$TC_BZ2 -C $TCDIR/ --strip-components=1;

do_okay "Extracted $TC_NAME to $TCDIR"
exit 0;
