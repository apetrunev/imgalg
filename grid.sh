#!/bin/sh

OPTS=$(getopt -o i:o: -l spacing -l color -l opacity: -l thickness: -- "$@")

IMG=
OUT=
SPACING=
COLOR=
THICKNESS=
OPACITY=

eval set -- "$OPTS"

while [ $# -gt 0 ]; do
	case "$1" in
	-i)
		IMG=$2
		shift;;
	-o)
		OUT=$2
		shift;;
	--spacing)
		SPACING=$2
		shift;;
	--color)
		COLOR=$2
		shift;;
	--thickness)
		THICKNESS=$2
		shift;;
	--opacity)
		OPACITY=$2
		shift;;
	--)
		shift
		break;;
	-*)
		echo "error: unrecongized option" 1>&2
		break;;
	*)
		break;;
	esac
	shift
done 

[ -n "$IMG" ] || exit 1
[ -n "$OUT" ] || exit 1
[ -n "$SPACING" ] || SPACING="100"
[ -n "$COLOR" ] || COLOR="red"
[ -n "$THICKNESS" ] || THICKNESS="1"
[ -n "$OPACITY" ] || OPACITY="1"

WIDTH=$(identify -format %w "$IMG")
HEIGHT=$(identify -format %h "$IMG")
WIDX=$(expr $WIDTH - 1)
HIDX=$(expr $HEIGHT - 1)

#
# define paths for horizontal lines 
#

HLINES=""
for IDX in $(seq 0 $SPACING $HIDX); do
	HLINES="$HLINES M 0,$IDX L $WIDX,$IDX"
	IDX=$(expr $IDX + $SPACING)
done

#
# do the same thing for vertical lines
#

VLINES=""
for IDX in $(seq 0 $SPACING $WIDX); do
	VLINES="$VLINES M $IDX,0 L $IDX,$HIDX"
	IDX=$(expr $IDX + $SPACING)
done

#
# define coordinates for labels
#

TEXT=""
for IDX in $(seq 0 $SPACING $WIDX); do
	TEXT="$TEXT text 0,$IDX '$IDX'"
done

for IDX in $(seq 0 $SPACING $WIDX); do
	TEXT="$TEXT text $IDX,24 '$IDX'"
done

OPTIONS="-fill none -stroke $COLOR -strokewidth $THICKNESS"

convert $IMG $OPTIONS -pointsize 24 -draw "$TEXT stroke-opacity $OPACITY path '$VLINES' path '$HLINES'" $OUT
