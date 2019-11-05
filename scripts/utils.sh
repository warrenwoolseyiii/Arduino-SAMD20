################################ Message Strings ###############################
_DBG_EN=0; _PYENV=0;
_EXIT_MSG='\nFatal Error. Exiting.\n';
################################# Color Methods #################################
_pmod=0; _vmod=0; _vmodbuff='';
### base methods (normal, light/pastel)
paint() { local c=1; local _vb=''; case "$1" in "nrm") c=0;; "gry") c=30;; "red") c=31;; "grn") c=32;; "ylw") c=33;; "blu") c=34;; "pur") c=35;; "aqu") c=36;; "wht") c=1;; *) echo "inv color"; esac;
if [[ $_vmod -eq 0 ]]; then printf "\\033[$_pmod;$c""m$2""\033[m"; else printf -v _vb "\\033[$_pmod;$c""m$2""\033[m"; _vmodbuff+=$_vb; fi; _pmod=0; _vmod=0; }
lpaint() { local c=1; case "$1" in "nrm") c=0;; "gry") c=90;; "red") c=91;; "grn") c=92;; "ylw") c=93;; "blu") c=94;; "pur") c=95;; "aqua") c=96;; "wht") c=97;; *) echo "inv color"; esac;
if [[ $_vmod -eq 0 ]]; then printf "\\033[$_pmod;$c""m$2""\033[m"; else printf -v _vb "\\033[$_pmod;$c""m$2""\033[m"; _vmodbuff+=$_vb; fi; _pmod=0; _vmod=0; }
### macros - normal
paintln() { paint "$1" "$2\n"; };
paintul() { _pmod=4; paint "$1" "$2"; }; paintulln() { _pmod=4; paintln "$1" "$2"; };
painthi() { _pmod=7; paint "$1" "$2"; }; painthiln() { _pmod=7; paintln "$1" "$2"; };
### macros - light
lpaintln() { lpaint "$1" "$2\n"; };
lpaintul() { _pmod=4; lpaint "$1" "$2"; }; lpaintulln() { _pmod=4; lpaintln "$1" "$2"; };
lpainthi() { _pmod=7; lpaint "$1" "$2"; }; lpainthiln() { _pmod=7; lpaintln "$1" "$2"; };
### macros - virtual
vpaintclear() { _vmod=0; _vmodbuff=''; }
vpaintflush() { _vmod=0; printf '%s\r' "$_vmodbuff"; _vmodbuff=''; };
vpaint() { _vmod=1; paint "$1" "$2"; }; vpaintln() { _vmod=1; paintln "$1" "$2"; };
vpaintul() { _vmod=1; paintul "$1" "$2"; }; vpaintulln() { _vmod=1; paintulln "$1" "$2"; };
vpainthi() { _vmod=1; painthi "$1" "$2"; }; vpainthiln() { _vmod=1; painthiln "$1" "$2"; };
### ------------------------------ extensions ------------------------------ ###
confirm() { local evln='paintln "red" "NO"; return 1;'; read -p "$1 (y/n) > " -r _choice;
case "$_choice" in [yY][eE][sS]|[yY]) evln='paintln "grn" "YES"; return 0;';; esac;
eval "$evln"; } ## usage: confirm 'Are you sure?'
should_exit() { if ! [[ "$1" -eq "0" ]]; then lpaint 'red' '[FATAL]'; printf " exit code $1. ($2)\n$_EXIT_MSG\n"; exit 2; fi; }
should_warn() { if ! [[ "$1" -eq "0" ]]; then paint 'ylw' '[WARN]'; printf "Got: $1\n"; return 1; else return 0; fi }
do_info() { paint 'wht' '[INFO]'; printf ' %s\n' "$1"; }
do_warn() { paint 'ylw' '[WARN]'; printf ' %s\n' "$1"; }
do_hint() { lpaint 'ylw' '[HINT]'; printf ' %s\n' "$1"; }
do_okay() { lpaint 'grn' '[OKAY]'; printf ' %s\n' "$1"; }
do_error() { lpaint 'red' '[ERROR]'; printf ' %s\n' "$1"; }
do_debug() { if [[ $_DBG_EN -eq 1 ]]; then lpaint 'blu' '[DEBUG]'; printf " $1\n"; fi; }
################################################################################
