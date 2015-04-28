#!/bin/bash

echo "shit" > dump.txt

output=$(ftm /tmp/__procdump.tm | egrep "OUT instruction" | egrep -o "[0-9]+")
echo "$output"
