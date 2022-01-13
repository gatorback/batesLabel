if [ $# != 1 ]; then
	echo "error: this script requires a single argument"
	exit 1
fi

script=`which "$0"`
scriptdir=`dirname "$script"`
# pdftk=$scriptdir/pdftk/bin/pdftk   #gatorback: install pdftk-java via homebrew instead

dir=`dirname "$1"`
name=`basename "$1" .pdf`
output=$dir/combined.pdf

##### use pdftk to create a new pdf or append to an existing pdf #####
if [ ! -e "$output" ]; then
	pdftk "$1" cat output "$output"
else
	pdftk "$output" "$1" cat output "$output-2"
	mv "$output-2" "$output"
fi
