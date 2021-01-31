#!/bin/bash

ki()
{
    echo -e "\nClosing mudpi..."   
    /usr/bin/tmux kill-session -t botty
    PGID=$(ps -o pgid= $$ | grep -o [0-9]*)
    kill -SIGTERM -"$PGID"
}

trap ki SIGINT
echo "Starting mudpi..."
cd EmberMUD/src
./startup 20495 &
cd ../../ttyd/build
./ttyd -p 16377 -t cursorBlink=true -t titleFixed=Mudpi ../../tintin/start_tt.sh >/dev/null 2>/dev/null&
cd ../..
sleep 2
/usr/bin/tmux new -d -s botty EmberMUD/bot/botty.py &

wait