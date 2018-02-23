#!/bin/bash
output=output.log

test()
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
test "30 0 0.3 1000 3" "flip.log"

echo "TEST LOSS"
test "30 0.1 0 1000 3" "loss.log"

echo "TEST ARQ"
test "30 0.1 0.3 1000 3" "arq.log"
