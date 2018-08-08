#!/bin/bash

./rev2_modify << "EOF"
A
EOF

for i in {a,b,c}
do
    echo "$i" + " "
done
