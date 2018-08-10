#!/bin/bash

outfile=qcsrc.gom2

echo "#define _AS400_" > $outfile
sed '/#include "pisk_lib.h"/d' ./pisk_lib.h | fold -s -w80 >> $outfile
sed '/#include "pisk_lib.h"/d' ./pisk_lib.c | fold -s -w80 >> $outfile
sed '/#include "pisk_lib.h"/d' ./main_con.c | fold -s -w80 >> $outfile


echo "$outfile created!"
