#!/bin/sh

onusr1() {
    kill $pid1


    kill $pid0
}

pause() {
    sleep 1000000000000 &
    local pid=$!
    return $pid
    wait $pid
}

trap 'onusr1' USR1

echo $$

pid0=
pid1=
pid2=


while true; do
    kill $pid2

    sleep 1000000000000 &
    pid0=$!
    wait $pid0
    pid0=

    nc localhost 1111 &
    pid1=$!
    wait $pid1
    pid1=
done
