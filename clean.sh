#!/bin/bash

DIR="$HOME"
FIFO1="/tmp/fifo1"
FIFO2="/tmp/fifo2"

if [[ ! -d $DIR ]];
then
	mkdir -pv "$HOME"/myDir/{a,b,c,d,e,}/{aa,bb,cc,dd,ee,}
	for dir in $(find "$HOME"/myDir -type d) ; do echo "$(date +%s)" > "$dir"/sendme_"$(date +%N)" ; done
	echo "$(date +%s)" > "$HOME"/myDir/sendme_"$(date +%N)"
fi

if [ -p $FIFO1 ] || [ -p $FIFO2 ];
then
	rm $FIFO1 && rm $FIFO2
	echo "fifo removed succesfully!"
fi

ipcrm -a
echo "ipcs removed succesfully!"

