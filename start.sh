#!/bin/bash

port=${1:-16377}

ki()
{
    echo -e "\nClosing mudpi..."   
    PGID=$(ps -o pgid= $$ | grep -o [0-9]*)
    kill -SIGTERM -"$PGID"
}

trap ki SIGINT
cd EmberMUD/src
./startup 20495 &
echo "Started EmberMUD on port 20495"
cd ../../ttyd/build
./ttyd -p "$port" -t cursorBlink=true -t titleFixed=Mudpi ../../tintin/start_tt.sh >/dev/null 2>/dev/null&
echo "Started ttyd on port $port"
cd ../..
sleep infinity
