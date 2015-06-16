#!/bin/sh -e 

OPTS=$(getopt -o x:y: -l rmin: -l rmax: -l step: -- "$@")

X=""
Y=""
RMIN=""
RMAX=""
STEP=""

eval set -- "$OPTS"

while [ $# -gt 0 ]; do
	case "$1" in
	-x)
		X=$2
		shift;;	
	-y)
		Y=$2
		shift;;
	--rmin)
		RMIN=$2
		shift;;
	--rmax)
		RMAX=$2
		shift;;
	--step)
		STEP=$2
		shift;;
	--)
		shift
		break;;
	-*) 
		echo "error: unrecognized option" 1>&2
		exit 1
	(*)
		break;;
	esac
	shift
done

[ -n "$X" ] || exit 1
[ -n "$Y" ] || exit 1
[ -n "$RMIN" ] || exit 1
[ -n "$RMAX" ] || exit 1

for R in $(seq $RMIN $STEP $RMAX); do
	echo "$X $Y $R"
done

