#! /bin/bash

sub_topic=pong
pub_topic=ping
n=0
nmax=3

while true
do
    msg=$(mosquitto_sub -t ${sub_topic} -C 1)
    #echo "${sub_topic}: $msg"
    let n=n+1
    test $n -ge $nmax && break
    mosquitto_pub -t ${pub_topic} -m "{\"event\":{\"name\":\"${pub_topic}\", \"data\":$n}}"
done
