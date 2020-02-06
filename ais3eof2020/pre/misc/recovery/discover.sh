#!/bin/sh

for f in part1/*; do
    ff=${f#part1/}
    cat part1/$ff | base64 --decode | base64 --decode > dest1/$ff
done

for f in part2/*; do
    ff=${f#part2/}
    cat part2/$ff | base64 --decode | base64 --decode | base64 --decode > dest2/$ff
done
