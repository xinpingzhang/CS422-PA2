#!/bin/bash
output=output.log

echo "10 0 0.3 1000 3" | ./bin/prog2_arq > $output
cat $output | grep -a "MAINLOOP: data given to student" | sed "$ d" | awk '{print $6}' > given.txt
cat $output | grep -a "TOLAYER5" | awk '{print $4}' > received.txt
diff given.txt received.txt
