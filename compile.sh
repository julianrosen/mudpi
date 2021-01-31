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