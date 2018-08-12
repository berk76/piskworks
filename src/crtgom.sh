#!/bin/sh

# This script will create source file for AS400.
# Read README.md for more information.

outfile=qcsrc.gom

echo "#define _AS400_" > $outfile
echo "#define _EINSTEIN_" >> $outfile
echo " " >> $outfile

sed 's/\r//' res_con.h | indent -st -l75 >> $outfile
sed 's/\r//' pisk_lib.h | indent -st -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' pisk_lib.c | sed 's/\r//' | indent -st -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' main_con.c | sed '/#include "res_con.h"/d' | sed 's/\r//' | indent -st -l75 >> $outfile

echo "$outfile created!"
