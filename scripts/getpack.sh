#!/bin/bash

ARGS=("$@");
ARGC=$#;
THISDIR=$(dirname `realpath $0`);
DLDIR=$(realpath $THISDIR/../dl);
DPDIR=$(realpath $THISDIR/../devicepack);

source $THISDIR/utils.sh;
! [[ $? -eq 0 ]] && echo "FATAL: Unable to source $THISDIR/utils.sh" && exit 1;

main() {
	# Give names to stdin args
	local BASE_URL="${ARGS[0]}";
	local VENDOR="${ARGS[1]}";
	local DFP_NAME="${ARGS[2]}";
	local DFP_VER="${ARGS[3]}";
	local ZIP_NAME="$VENDOR.$DFP_NAME.$DFP_VER.atpack";
	# See if target extract directory exists already
	do_info "Checking for device pack: $VENDOR.$DFP_NAME.$DFP_VER";
	if [[ -d $DPDIR/$VENDOR/$DFP_NAME/$DFP_VER ]]; then
		do_warn "Skipping as $DPDIR/$VENDOR/$DFP_NAME/$DFP_VER already exists";
		return 0;
	fi
	# Download zip file if DNE
	mkdir -p dl;
	if ! [[ -f dl/$ZIP_NAME ]]; then
		do_info "Downloading $ZIP_NAME from $BASE_URL";
		wget "$BASE_URL$ZIP_NAME" -O dl/$ZIP_NAME;
		! [[ $? -eq 0 ]] && do_error "failed to download $BASE_URL$ZIP_NAME" && return 1;
	fi
	# Create version directory and extract contents
	do_info "Extracting $ZIP_NAME";
	rm -rf $DLDIR/$DFP_VER;
	mkdir -p $DLDIR/$DFP_VER;
	cp $DLDIR/$ZIP_NAME $DLDIR/$DFP_VER/$ZIP_NAME;
	cd $DLDIR/$DFP_VER; unzip -q $ZIP_NAME; rm $ZIP_NAME;
	# Create target dir and move extracted version dir inside
	mkdir -p $DPDIR/$VENDOR/$DFP_NAME
	mv $DLDIR/$DFP_VER $DPDIR/$VENDOR/$DFP_NAME/
	! [[ $? -eq 0 ]] && do_error "failed to move $DLDIR/$DFP_VER" && return 1;
	do_okay "Successfully extracted $ZIP_NAME to $DPDIR/$VENDOR/$DFP_NAME/$DFP_VER";
	return 0;
}

main;
exit $?;
