#!/bin/bash
sleep 0.1 # Workaround for some strange bug with ttyd :\

if [ "$#" -eq 0 ]
then
    /home/mudpi/mudpi/tintin/src/tt++ -G /home/mudpi/mudpi/tintin/mud_config -e "#var username EMPTY" -e "#var password EMPTY"
elif [ "$#" -eq 1 ]
then
    /home/mudpi/mudpi/tintin/src/tt++ -G /home/mudpi/mudpi/tintin/mud_config -e "#var username $1" -e "#var password EMPTY"
else
    /home/mudpi/mudpi/tintin/src/tt++ -G /home/mudpi/mudpi/tintin/mud_config -e "#var username $1" -e "#var password $2"
fi

#echo "Connection closed"
