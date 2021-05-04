#!/bin/bash

ARGS=("$@");
ARGC=$#;
THISDIR=$(dirname `realpath $0`);
PROJ_ROOT=$(realpath $THISDIR/../)

source $THISDIR/utils.sh;
! [[ $? -eq 0 ]] && echo "FATAL: Unable to source $THISDIR/utils.sh" && exit 1;

# Give names to stdin args
BASE_URL="${ARGS[0]}";
VENDOR="${ARGS[1]}";
DFP_NAME="${ARGS[2]}";
DFP_VER="${ARGS[3]}";
DL_DIR="${ARGS[4]}";
DP_DIR=$(realpath $DL_DIR/devicepack);
ZIP_NAME="$VENDOR.$DFP_NAME.$DFP_VER.atpack";

# See if target extract directory exists already
do_info "Checking for device pack: $VENDOR.$DFP_NAME.$DFP_VER";
if [[ -d $DP_DIR/$VENDOR/$DFP_NAME/$DFP_VER ]]; then
	do_warn "Skipping as $DP_DIR/$VENDOR/$DFP_NAME/$DFP_VER already exists";
	exit 0;
fi

# Ensure download directory is writeable
if ! [[ -w $DL_DIR ]]; then do_error "Error: path $DL_DIR not writeable"; exit 1; fi

# Create devicepack dir if DNE
mkdir -p $DP_DIR;

# Download zip file if DNE
if ! [[ -f $DL_DIR/$ZIP_NAME ]]; then
	do_info "Downloading $ZIP_NAME from $BASE_URL";
	wget "$BASE_URL$ZIP_NAME" -O $DL_DIR/$ZIP_NAME;
	! [[ $? -eq 0 ]] && do_error "failed to download $BASE_URL$ZIP_NAME" && exit 1;
fi

# Create version directory and extract contents
do_info "Extracting $ZIP_NAME";

# Create a version dir that will be moved later
rm -rf $DL_DIR/$DFP_VER;
mkdir -p $DL_DIR/$DFP_VER;

# Copy zip into version dir, extract, remove copied zip
cp $DL_DIR/$ZIP_NAME $DL_DIR/$DFP_VER/$ZIP_NAME;
cd $DL_DIR/$DFP_VER; unzip -q $ZIP_NAME; rm $ZIP_NAME;

# Create target dir and move extracted version dir inside
mkdir -p $DP_DIR/$VENDOR/$DFP_NAME
mv $DL_DIR/$DFP_VER $DP_DIR/$VENDOR/$DFP_NAME/
! [[ $? -eq 0 ]] && do_error "failed to move $DL_DIR/$DFP_VER" && exit 1;

# Cleanup
rm -rf $DL_DIR/$DFP_VER;
do_okay "Successfully extracted $ZIP_NAME to $DP_DIR/$VENDOR/$DFP_NAME/$DFP_VER";

exit 0;
