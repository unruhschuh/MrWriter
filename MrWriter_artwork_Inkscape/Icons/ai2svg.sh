#!/bin/bash
for filename in *.ai; do
    filenameSansExt="${filename%.*}"
    inkscape --file="$filename" --export-plain-svg="$filenameSansExt.svg"
done
