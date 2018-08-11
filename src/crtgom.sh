#!/bin/bash

outfile=qcsrc.gom

echo "#define _AS400_" > $outfile
echo " " >> $outfile

sed '/#include "pisk_lib.h"/d' pisk_lib.h | sed 's/\r//' | indent -st -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' pisk_lib.c | sed 's/\r//' | indent -st -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' main_con.c | sed 's/\r//' | indent -st -l75 >> $outfile

echo "$outfile created!"

