if [ $# != 1 ]; then
	echo "error: this script requires a single argument"
	exit 1
fi

script=`which "$0"`
scriptdir=`dirname "$script"`
# pdftk=$scriptdir/pdftk/bin/pdftk #gatorback: install pdftk-java via homebrew instead

dir=`dirname "$1"`
name=`basename "$1" .pdf`
output=$dir/$name-cleaned.pdf

##### clean the pdf by doing a simple I/O through pdftk #####
pdftk "$1" output "$output"

