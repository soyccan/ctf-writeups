#!/bin/sh

trap 'kill $pid1 ; sleep 0.25 ; kill $pid0' USR1

echo $$

while true; do
    sleep infinity &
    pid0=$!
    wait $pid0
#     python example-exp.py
    python exp.py &
    pid1=$!
done
