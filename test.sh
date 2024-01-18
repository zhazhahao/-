#!/bin/bash

clear
filename=""
size=""
old_size=""
change_count=0
loop_count=0
echo "Input Filename："
read filename
if [ ! -f "$filename" ]; then
    echo "Can not find the file[$filename]"
    exit 1
fi
size=$(ls -l "$filename" | awk '{print $5}')
while true; do
    loop_count=$((loop_count+1))
    if [ "$loop_count" -ge 11 ]; then
        echo " 10 times not changed，test end！"
        break
    fi


    sleep 2


    new_size=$(ls -l "$filename" | awk '{print $5}')

    echo "test file's status"
    if [ "$new_size" = "$size" ]; then
        echo "not changed"
        continue

    else
        old_size="$size"
        size="$new_size"
        change_count=$((change_count+1))
        echo "file [ $filename ] size changed"

        if [ "$change_count" -ge 2 ]; then
            echo "change number exceed $change_count ，test end"
            break
        fi
    fi


   
done


exit 0
