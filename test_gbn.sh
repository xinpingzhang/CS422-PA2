#!/bin/bash
output=output.log

test_gbn()
{
	echo $1 | ./bin/prog2_gbn > $2
	cat $2 | grep -a "MAINLOOP: data given to student" | sed "$ d" | awk '{print $6}' > given.txt
	cat $2 | grep -a "TOLAYER5" | awk '{print $4}' > received.txt
	if diff given.txt received.txt; then
		echo "OK!"
		rm given.txt received.txt
		rm $2
	else
		echo "ERROR!"
		exit 1
	fi
}

echo "TEST DUM"
test_gbn "100 0 0 1000 3" "dum.log"

echo "TEST FLIP"
test_gbn "20 0 0.2 1000 3" "flip.log"

echo "TEST LOSS"
test_gbn "20 0.2 0 1000 3" "loss.log"

echo "TEST ARQ"
test_gbn "20 0.2 0.2 1000 3" "gbn.log"
