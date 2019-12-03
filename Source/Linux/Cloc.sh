#!/bin/bash

CLOCHEADER=$1
PROJECT=`echo $2 | awk '{print toupper( $0 )}'`

cloc --exclude-dir=Libs --exclude-lang="Bourne Shell","make","XSD","vim script" --csv --out=cloc.txt ../
grep 1 cloc.txt | cut -d, -f5 > cloc_lines.txt
CLOC_CXX=`head -2 cloc_lines.txt | tail -1 | cut -d, -f5`
CLOC_HXX=`head -3 cloc_lines.txt | tail -1 | cut -d, -f5`
CLOC_TOTAL=$((${CLOC_CXX}+${CLOC_HXX}))

rm -rf cloc.txt
rm -rf cloc_lines.txt

printf "#ifndef __%s_CLOC_HPP__\n" $PROJECT > $CLOCHEADER
printf "#define __%s_CLOC_HPP__\n\n" $PROJECT >> $CLOCHEADER

printf "#define CLOC_LINECOUNT	%d\n\n" $CLOC_TOTAL >> $CLOCHEADER

printf "#endif\n" >> $CLOCHEADER

