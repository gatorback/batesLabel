if [ $# != 1 ]; then
	echo "error: this script requires a single argument"
	exit 1
fi

script=`which "$0"`
scriptdir=`dirname "$script"`
# pdftk=$scriptdir/pdftk/bin/pdftk #gatorback: install pdftk-java via homebrew instead
generate=$scriptdir/labeling/generate-labeling-pdf
multistamp=$scriptdir/labeling/labeling.pdf
data=$scriptdir/labeling/labeling.txt

dir=`dirname "$1"`
name=`basename "$1" .pdf`
output=$dir/$name-bates.pdf

##### dump pdf data to so generate can determine number of pages #####
pdftk "$1" dump_data > "$data"

##### generate labeling pdf to overlay #####
"$generate" "$data" "$multistamp"
pdftk "$multistamp" output "$multistamp-compressed" compress   #gatorback: invoke : pdftk-java, installed via homebrew instead  
mv "$multistamp-compressed" "$multistamp"

##### overlay labeling pdf on input pdf #####
pdftk "$1" multistamp "$multistamp" output "$output"

