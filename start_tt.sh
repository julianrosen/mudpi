#!/bin/bash
sleep 0.1 # Workaround for some strange bug with ttyd :\
/home/mud/newpi/tintin/src/tt++ -G /home/mud/newpi/tintin/mud_config
echo "Connection closed"
