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

echo Done.
