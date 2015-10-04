#!/bin/bash
for filename in *.svg; do
    filenameSansExt="${filename%.*}"
    inkscape --file="$filename" --export-width=48 --export-height=48 --export-png="png/${filenameSansExt}_48.png"
    inkscape --file="$filename" --export-width=32 --export-height=32 --export-png="png/${filenameSansExt}_32.png"
    inkscape --file="$filename" --export-width=24 --export-height=24 --export-png="png/${filenameSansExt}_24.png"
done
