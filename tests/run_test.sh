#!/bin/sh

for var in *.sce
do
        ./unittest $var ${var/.sce/.exp}
done

echo "All tests done!"
exit 0
