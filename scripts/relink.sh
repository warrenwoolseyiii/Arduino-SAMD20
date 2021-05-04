#!/bin/bash

THISDIR=$(dirname `realpath $0`);
TC_NAME="$1";
DL_DIR="$2";

TC_DIR=$(realpath $THISDIR/..)/toolchain;
TC_SAVEDIR=$DL_DIR/toolchains;

DP_DIR=$(realpath $DL_DIR/devicepack);

source $THISDIR/utils.sh;
! [[ $? -eq 0 ]] && echo "FATAL: Unable to source $THISDIR/utils.sh" && exit 1;

################################## Toolchain ###################################
# Check that toolchain dir is symlinked
if ! [[ -L $TC_DIR ]] && [[ -d $TC_DIR ]]; then
	do_warn "Removing $TC_DIR as it is not symlinked";
	rm -rf $TC_DIR;
fi

# If toolchain is a symlink that exists, check if the linked path matches TC_NAME
# If so, exit. If not, unlink it
if [[ -L $TC_DIR ]]; then
	curpath=$(realpath $TC_DIR)
	if [[ -d $curpath ]]; then
		curname=$(basename $curpath)
		if ! [[ "$curname" = "$TC_NAME" ]]; then
			do_info "Unlinking current toolchain $curname"
			unlink $TC_DIR;
		else
			do_okay "Found $TC_NAME";
		fi
	else
		# link exists, but dest does not. unlink
		unlink $TC_DIR;
	fi
fi

# Link extracted toolchain to TC_DIR
ln -s $TC_SAVEDIR/$TC_NAME $TC_DIR
! [[ $? -eq 0 ]] && do_error "error linking $TC_DIR" && exit 1;
do_okay "Toolchain $TC_NAME linked to $TC_DIR"

################################## Devicepack ##################################
[[ -L devicepack ]] && unlink devicepack;
[[ -d devicepack ]] && rm -rf devicepack;

# Link DP_DIR to devicepack
ln -s $DP_DIR devicepack
! [[ $? -eq 0 ]] && do_error "error linking $DP_DIR" && exit 1;
do_okay "devicepack linked to $DP_DIR"



exit 0;
