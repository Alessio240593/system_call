#!/bin/bash

DIR="$HOME"
FIFO1="/tmp/fifo1"
FIFO2="/tmp/fifo2"



# if [[ ! -d $DIR ]] ; then
	index=0
	for i in {1..23} ; do
		# printf "i : %d\n" "$i"
		if [[ $(($i % 2)) == 0 ]] ; then
			printf "ABCDEFGHIL" > "$HOME"/myDir/sendme_"$i"
		else
			printf "ABCDEFGHIL" > "$HOME"/myDir/sendme_"$i".txt
		fi
		index=$(($index + 1))
	done	
	# mkdir -pv "$HOME"/myDir/{a,b,c,d,e,}/{aa,bb,cc,dd,ee,}
	# for dir in $(find "$HOME"/myDir -type d) ; do echo "$(date +%s)" > "$dir"/sendme_"$(date +%N)" ; done
	# echo "$(date +%s)" > "$HOME"/myDir/sendme_"$(date +%N)"
# fi

if [ -p $FIFO1 ] || [ -p $FIFO2 ];
then
	rm $FIFO1 && rm $FIFO2
	echo "fifo removed succesfully!"
fi

ipcrm -a
echo "ipcs removed succesfully!"

