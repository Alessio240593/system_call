#!/bin/bash

DIR="/tmp/myDir"
FIFO1="/tmp/myDir/fifo1"
FIFO2="/tmp/myDir/fifo2"

if [[ ! -d $DIR ]];
then
	mkdir -pv /tmp/myDir/{a,b,c,d,e,}/{aa,bb,cc,dd,ee,}
	for dir in $(find /tmp/myDir -type d) ; do echo "$(date +%s)" > "$dir"/sendme_"$(date +%N)" ; done
	echo "$(date +%s)" > /tmp/myDir/sendme_"$(date +%N)"
fi

if [ -p $FIFO1 ] || [ -p $FIFO2 ];
then
	rm $FIFO1 && rm $FIFO2
	echo "fifo removed succesfully!"
fi

ipcrm -a
echo "ipcs removed succesfully!"

