#!/bin/sh

IMG=$1
OUTPUT="grid-$IMG"

SPACING="100"
COLOR="red"
THICKNESS="1"
OPACITY="1"

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

convert $IMG $OPTIONS -pointsize 24 -draw "$TEXT stroke-opacity $OPACITY path '$VLINES' path '$HLINES'" $OUTPUT
