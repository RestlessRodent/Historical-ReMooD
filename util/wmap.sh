#!/bin/sh

rm -rf WHITECHAR BCHAR

# Convert red to white
mkdir -p WHITECHAR
for file in ufnr00[0123456]?.ppm; do convert -fuzz "25%" -fill white -opaque red "$file" "WHITECHAR/$file"; done

# Convert cyan to black
mkdir -p BCHAR
for file in WHITECHAR/*.ppm; do convert -fuzz "25%" -fill black -opaque "#00ffff" "$file" "BCHAR/`basename $file`"; done

# Append all images to a single row
convert BCHAR/*.ppm +append ROWS.ppm
