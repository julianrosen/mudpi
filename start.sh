#!/bin/bash
trap 'kill %1; kill %2; kill %3; tmux kill-session -t botty' SIGINT
(cd EmberMUD/src && ./startup 20495) &\
sleep 1; /usr/bin/tmux new -s botty -d EmberMUD/bot/botty.py &\
(cd ttyd/build && ./ttyd -p 16377 -t cursorBlink=true -t titleFixed=Mudpi ../../tintin/start_tt.sh)

echo "ALL DONE"