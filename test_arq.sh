#!/bin/bash
output=output.log

test_arq()
{
	echo $1 | ./bin/prog2_arq > $2
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

echo "TEST FLIP"
test_arq "20 0 0.3 2000 3" "flip.log"

echo "TEST LOSS"
test_arq "20 0.1 0 2000 3" "loss.log"

echo "TEST ARQ"
test_arq "20 0.1 0.3 2000 3" "arq.log"
