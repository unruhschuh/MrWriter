#!/bin/bash
echo $PWD
for filename in *.svg; do
    filenameSansExt="${filename%.*}"
    inkscape --file="$PWD/$filename" --export-width=48 --export-height=48 --export-png="$PWD/png/${filenameSansExt}_48.png"
    inkscape --file="$PWD/$filename" --export-width=32 --export-height=32 --export-png="$PWD/png/${filenameSansExt}_32.png"
    inkscape --file="$PWD/$filename" --export-width=24 --export-height=24 --export-png="$PWD/png/${filenameSansExt}_24.png"
done
