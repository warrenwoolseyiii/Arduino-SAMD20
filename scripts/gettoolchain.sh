#!/bin/bash

case "$1" in
	"7-2018-q2-update")
	TC_URL_LINUX='https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2?revision=bc2c96c0-14b5-4bb4-9f18-bceb4050fee7?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,7-2018-q2-update';
	TC_URL_MAC='https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-mac.tar.bz2?revision=982ef8a4-1815-4651-9c44-6144c9d8b34b?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Mac%20OS%20X,7-2018-q2-update';
	;;
	"8-2019-q3-update")
	TC_URL_LINUX='https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2?revision=c34d758a-be0c-476e-a2de-af8c6e16a8a2?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,8-2019-q3-update';
	TC_URL_MAC='https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-mac.tar.bz2?revision=6a06dd2b-bb98-4708-adac-f4c630c33f4f?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Mac%20OS%20X,8-2019-q3-update';
	;;
	"9-2019-q4-major")
	TC_URL_LINUX='https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/RC2.1/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2?revision=6e63531f-8cb1-40b9-bbfc-8a57cdfc01b4&la=en&hash=F761343D43A0587E8AC0925B723C04DBFB848339';
	TC_URL_MAC='https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/RC2.1/gcc-arm-none-eabi-9-2019-q4-major-mac.tar.bz2?revision=0108cc32-e125-409b-ae7b-b2d6d30bf69c&la=en&hash=8C90ACFF11212E0540D74DA6A4F6CEE7253CD13F';
	;;
	*) echo "Error: invalid arg \"$1\". Must be '7-2018-q2-update' or '8-2019-q3-update' or '9-2019-q4-major'." && exit 1;;
esac

case "$2" in
	"linux") TC_URL=$TC_URL_LINUX;;
	"mac") TC_URL=$TC_URL_MAC;;
	*) echo "Error: invalid arg \"$2\". Must be 'mac' or 'linux'." && exit 1;;
esac
TC_NAME="$1";
TC_EXTRACTDIR="gcc-arm-none-eabi-$1-$2";
TC_BZ2="gcc-arm-none-eabi-$1-$2.tar.bz2";

DL_DIR=$(realpath $THISDIR/../dl);
case "$3" in
	"") echo "Error: invalid DL_DIR. Set in config.mk." && exit 1;;
	*) DL_DIR=$3;
	if ! [[ -d $3 ]]; then
		mkdir $3;
		if ! [[ $? -eq 0 ]]; then echo "Unable to create dir $3"; exit 1; fi;
	fi
esac
if ! [[ -w $DL_DIR ]]; then echo "Error: path $DL_DIR not writeable"; exit 1; fi

THISDIR=$(dirname `realpath $0`);
TC_DIR=$(realpath $THISDIR/..)/toolchain;
TC_SAVEDIR=$DL_DIR/toolchains;

source $THISDIR/utils.sh;
! [[ $? -eq 0 ]] && echo "FATAL: Unable to source $THISDIR/utils.sh" && exit 1;

# Create directory to hold all toolchains
mkdir -p $TC_SAVEDIR
! [[ -d $TC_SAVEDIR ]] && do_error "failed to create TC_SAVEDIR=$TC_SAVEDIR" && exit 1;

# Download and Extract toolchain if DNE
if ! [[ -d $TC_SAVEDIR/$TC_NAME ]]; then
	# Download bz2 file if DNE
	if ! [[ -f $DL_DIR/$TC_BZ2 ]]; then
		do_info "Downloading $TC_BZ2 from $TC_URL";
		wget "$TC_URL" -O $DL_DIR/$TC_BZ2;
		! [[ $? -eq 0 ]] && do_error "failed to download $TC_BZ2" && exit 1;
	fi

	# Create and Extract to savedir/name
	mkdir $TC_SAVEDIR/$TC_NAME
	! [[ -d $TC_SAVEDIR/$TC_NAME ]] && do_error "failed to create $TC_SAVEDIR/$TC_NAME" && exit 1;

	tar xjf $DL_DIR/$TC_BZ2 -C $TC_SAVEDIR/$TC_NAME/ --strip-components=1;
	_code=$?;
	! [[ $_code -eq 0 ]] && do_error "got exit code $_code when extracting $DL_DIR/$TC_BZ2" && exit 1;
	do_okay "Extracted $TC_EXTRACTDIR to $TC_SAVEDIR/$TC_NAME";
fi

exit 0;
