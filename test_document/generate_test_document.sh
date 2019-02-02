#!/bin/sh

if [ "$#" -ne 1 ]
then
  N=1
else
  if [ $1 -lt 0 ]
  then
    N=1
  else
    N=$1
  fi
fi

filename=Testdoc.nogit.moj
filename_all_colors=Testdoc_all_colors.nogit.moj
filename_with_images=Testdoc_with_images.nogit.moj

echo Generating test document with $N pages ...

cat parts/header.part.moj > "$filename"
for (( i=1; i<=$N; i++))
do
  cat parts/page.part.moj >> "$filename"
done
cat parts/footer.part.moj >> "$filename"

echo Gzipping ...
gzip "$filename"

echo Renaming ...
mv "$filename.gz" "$filename"


cat parts/header.part.moj > "$filename_all_colors"
for (( i=1; i<=$N; i++))
do
  cat parts/page_all_colors.part.moj >> "$filename_all_colors"
done
cat parts/footer.part.moj >> "$filename_all_colors"

echo Gzipping ...
gzip "$filename_all_colors"

echo Renaming ...
mv "$filename_all_colors.gz" "$filename_all_colors"

echo Done.


cat parts/header.part.moj > "$filename_with_images"
for (( i=1; i<=$N; i++))
do
  cat parts/page_with_images.part.moj >> "$filename_with_images"
done
cat parts/footer.part.moj >> "$filename_with_images"

echo Gzipping ...
gzip "$filename_with_images"

echo Renaming ...
mv "$filename_with_images.gz" "$filename_with_images"

echo Done.
