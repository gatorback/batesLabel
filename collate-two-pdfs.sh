if [ $# != 2 ]; then
	echo "error: this script requires two arguments"
	exit 1
fi

script=`which "$0"`
scriptdir=`dirname "$script"`
# pdftk=$scriptdir/pdftk/bin/pdftk #gatorback: install pdftk-java via homebrew instead

dir=`dirname "$1"`
name=`basename "$1" .pdf`
name2=`basename "$2" .pdf`
output=$dir/$name-$name2-collated.pdf

##### collate the pdf through pdftk #####
"$pdftk" A="$1" B="$2" shuffle A B output "$output"

