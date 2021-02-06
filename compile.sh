#!/bin/bash

# Compile EmberMUD
cd EmberMUD/src
make
cd ../..

# Compile TinTin
cd tintin/src
./configure
make
cd ../..

# Compile ttyd
mkdir ttyd/build
cd ttyd/build
cmake ..
make
cd ../..

# Create start script
echo '#!/bin/bash

port=${1:-16377}

ki()
{
    echo -e "\nClosing mudpi..."   
    /usr/bin/tmux kill-session -t botty
    PGID=$(ps -o pgid= $$ | grep -o [0-9]*)
    kill -SIGTERM -"$PGID"
}

trap ki SIGINT
echo "Starting mudpi on port $port..."
cd EmberMUD/src
./startup 20495 &
cd ../../ttyd/build
./ttyd -p "$port" -t cursorBlink=true -t titleFixed=Mudpi ../../tintin/start_tt.sh >/dev/null 2>/dev/null&
cd ../..
sleep 2
/usr/bin/tmux new -d -s botty EmberMUD/bot/botty.py &' > start.sh
chmod +x start.sh


# Create systemd service
my_dir=$(pwd)
echo "[Unit]
Description=Mudpi server
after=network.target

[Service]
User=mud
ExecStart=$my_dir/start.sh
WorkingDirectory=$my_dir
Restart=on-failure

[Install]
WantedBy=multi-user.target" > mudpi.service