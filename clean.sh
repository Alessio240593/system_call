#!/bin/bash

DIR="$HOME"
FIFO1="/tmp/fifo1"
FIFO2="/tmp/fifo2"

TXT="
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ut eleifend ligula, id iaculis leo. Sed vulputate nulla sodales pulvinar pharetra. Praesent bibendum ut mi sit amet vestibulum. Integer ultrices urna vel risus ultrices, et consectetur lectus auctor. Ut eget dictum elit, ac sodales nulla. Mauris fringilla est nisi, et vestibulum nibh vestibulum eu. Mauris scelerisque mi vel dolor lobortis luctus. Nulla euismod blandit finibus. In posuere elit et felis mollis, ut aliquam risus posuere.

Nunc at est enim. Curabitur faucibus accumsan bibendum. Proin lacinia placerat pretium. Maecenas rhoncus purus a lorem sagittis varius. Aenean elementum ipsum eu est commodo, non euismod erat aliquam. Aliquam a interdum lorem. Pellentesque ut orci dolor. Duis convallis facilisis ultricies.

Quisque eget rhoncus sapien. Pellentesque a ante tellus. Cras malesuada felis non ex posuere, sed facilisis felis sagittis. Vivamus eu efficitur nulla, id tempor lectus. Fusce sed ipsum convallis, pulvinar elit ac, commodo lectus. Quisque lacinia hendrerit consequat. Quisque orci velit, finibus at luctus eget, luctus sit amet dui. Suspendisse potenti. Praesent efficitur sem nibh, ac varius metus fermentum sit amet. In consequat pellentesque nunc, et rhoncus massa sodales sed. Suspendisse sit amet condimentum ligula. Duis porta est eu elit sodales, et lobortis ex semper. Maecenas sed enim a justo auctor hendrerit dictum eu massa. Maecenas interdum nulla id sem mollis varius. Mauris vitae lorem euismod, scelerisque justo quis, euismod mauris. Vestibulum vestibulum, enim quis cursus molestie, justo erat convallis purus, dignissim elementum quam mauris ut tellus.
"


# if [[ ! -d $DIR ]] ; then
	mkdir -pv "$HOME"/myDir

	index=0
	for i in {1..29} ; do
		# printf "i : %d\n" "$i"
		if [[ $(($i % 2)) == 0 ]] ; then
			printf "%s" "$TXT" > "$HOME"/myDir/sendme_"$i"
		else
			printf "%s" "$TXT" > "$HOME"/myDir/sendme_"$i".txt
		fi
		index=$(($index + 1))
	done	
	# mkdir -pv "$HOME"/myDir/{a,b,c,d,e,}/{aa,bb,cc,dd,ee,}
	# for dir in $(find "$HOME"/myDir -type d) ; do echo "$(date +%s)" > "$dir"/sendme_"$(date +%N)" ; done
	# echo "$(date +%s)" > "$HOME"/myDir/sendme_"$(date +%N)"
#fi

if [ -p $FIFO1 ] || [ -p $FIFO2 ];
then
	rm $FIFO1 && rm $FIFO2
	echo "fifo removed succesfully!"
fi

ipcrm -a
echo "ipcs removed succesfully!"

