Mudpi is a online multiplayer text-based RPG. The base game is EmberMUD, with a number of modifications
by JR. The text is fed through a modified version of the MUD client
TinTin++ (running server-side), and the result is served through a browser
using ttyd.

Compile the source using
```./compile.sh```. Then start the mudpi server on port <port> using ```./start <port>```.
Point your browser at `http://localhost:<port>/` and enjoy the game! The
script will also generate a systemd unit file mudpi.service, which you
can move to `/etc/systemd/system` if you want to start and stop mudpi
as a service.

I run Mudpi on a Raspberry Pi, but I have also tested it on Ubuntu.

<img src="mudpi.png" alt="Mudpi in action" width="800"/>
