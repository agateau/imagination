#!/bin/sh

sed -r -e "s/(.+)=\/.+(\/[^\/]+\....).*/\1=sample_ss\2/" $1 > tmp.imag
mv tmp.imag $2

exit

