#!/bin/bash
#: Title        : runmud.sh
#: Date         : Aug 24, 2010
#: Author       : "Bobby Bailey" <bobby@bucketmud.tk>
#: Version      : 1.0
#: Description  : Starts the BucketMUD server. When the BucketMUD server is
#:                shut down, this script will automatically attempt to restart
#:                the server unless the file $BUCKETMUD_HOME/area/shutdown.txt
#:                exists. If shutdown.txt exists, then the script will exit.
#:                All output from the server is redirected to the file
#:                $BUCKETMUD_HOME/log/bucketmud.log.


BUCKETMUD_HOME=/home/bucketmud/bucket/
port=1234

while true
do
    if [ -e $BUCKETMUD_HOME/area/shutdown.txt ]
    then
        rm $BUCKETMUD_HOME/area/shutdown.txt
    fi

    $BUCKETMUD_HOME/bin/bucketmud $port >> $BUCKETMUD_HOME/log/bucketmud.log 2>&1

    if [ -e $BUCKETMUD_HOME/area/shutdown.txt ]
    then
        exit 0
    fi

    sleep 5
done
