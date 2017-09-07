#! /bin/sh

HOST=`hostname`
IP=`getent hosts $HOST | awk '{print $1}'`

echo "IP = $IP"

j-greyd/j-greyd -s inet:2012@$HOST -a $IP


