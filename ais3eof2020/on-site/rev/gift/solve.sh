#!/bin/sh

zcat gift.gz > gift_0
chmod +x gift_0

for i in $(seq 0 5000); do
    echo $i
    cur=./gift_$i
    nxt=./gift_$(($i + 1))
    cat $cur | tail -c +$((0x8f9)) | head -c $((0x100)) | $cur | zcat > $nxt
    if [ $? = 1 ]; then
        cat $cur | tail -c +$((0x8f9)) | head -c $((0x100)) | $cur
        break
    fi
    chmod +x $nxt
done
