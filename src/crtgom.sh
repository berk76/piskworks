#!/bin/bash

outfile=qcsrc.gom2

echo "#define _AS400_" > $outfile
echo " " >> $outfile

sed '/#include "pisk_lib.h"/d' pisk_lib.h | indent -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' pisk_lib.c | indent -l75 >> $outfile
sed '/#include "pisk_lib.h"/d' main_con.c | indent -l75 >> $outfile

echo "$outfile created!"
