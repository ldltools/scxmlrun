#! /bin/bash

sub_topic=ping
pub_topic=pong
n=0
nmax=3

while true
do
    msg=$(mosquitto_sub -t ${sub_topic} -C 1)
    #echo "${sub_topic}: $msg"
    let n=n+1
    test $n -gt $nmax && break
    mosquitto_pub -t ${pub_topic} -m "{\"event\":{\"name\":\"${pub_topic}\", \"data\":$n}}"
done
