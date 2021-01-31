#!/bin/bash
sleep 0.1 # Workaround for some strange bug with ttyd :\
/home/mud/mudpi/tintin/src/tt++ -G /home/mud/mudpi/tintin/mud_config
echo "Connection closed"
