#!/bin/sh
n=5
if [ $# -gt 0 ]
  then
    n=$1
fi
for i in `seq $n`
do
    db=$(($RANDOM % 100))
    curl -i -H "Content-Type: application/json" -X POST -d '{"db": $db}' 127.0.0.1:5000/decibels
    sleep 1
done