#!/bin/sh

WD=$(dirname $(pwd))
SUBST1=$(echo 's#filename=(.+)#filename='"$WD"'/\1#')
SUBST2=$(echo 's#(music_[0-9]+)=(.+)#\1='"$WD"'/\2#')

echo $SUBST

if [ $# -lt 2 ]
then
	exit
fi

if [ $# -lt 3 ]
then
	sed -r -e "s/(.+)=\/.+(\/[^\/]+\....).*/\1=sample_ss\2/" $1 > tmp.imag
else
	sed -r -e $SUBST2 -e $SUBST1 $1 > tmp.imag
fi

mv tmp.imag $2

exit

