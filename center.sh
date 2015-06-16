#!/bin/sh

IMG=""
POINTS=""

while getopts "f:" opt; do
	case "$opt" in
	f)
		POINTS=$OPTARG
		;;
	*)
		echo "error: unknown option"
		;;
	esac
done 
shift $((OPTIND-1))

IMG=$1

[ -n "$POINTS" ] || exit 1
[ -n "$IMG" ] || exit 1

plot_dots() {
	cat $1 | gawk -v img=$2 '
	BEGIN {
		len = 1;
		strokewidth="0.5"
	}
	{
		x = $1
		y = $2
		r = $3
		
		x1 = x + r
		y1 = y + r
		
		xc = x + 2
		yc = y + 2

		drawc = " -draw \"circle " x "," y "," xc "," yc "\""
		draw = " -draw \"circle " x "," y "," x1 "," y1 "\""
		fill = " -fill \"None\""
		stroke = " -stroke \"blue\""
		strokewidth = " -strokewidth 4"

		plot[len++] = " convert " img " " fill " " stroke " " strokewidth " " draw " "  drawc " " img
	}
	END {
		for (i = 1; i < len; i++) {
			cmd = plot[i]
			print cmd | "/bin/sh" 
		}
	}'
}

plot_dots $POINTS $IMG
