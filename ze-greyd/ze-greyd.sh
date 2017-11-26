#! /bin/sh

HOST=`hostname`
IP=`getent hosts $HOST | awk '{print $1}'`

echo "IP = $IP"

ze-greyd/ze-greyd -s inet:2012@$HOST -a $IP


