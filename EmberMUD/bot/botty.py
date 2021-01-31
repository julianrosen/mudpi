#!/usr/bin/python3

import socket
import sys
from time import sleep
import subprocess
from time import perf_counter
from random import randint
#import chatbot
from collections import defaultdict
import signal
import select

def leave(signalnum,frame):
    print("Logging out...")
    try:
        write("ooc I'm outta here!")
        write("quit")
        print("::",wait_on_text("bye"))
    except:
        pass
    sys.exit(0)
    return None


signal.signal(signal.SIGHUP, leave)
signal.signal(signal.SIGINT, leave)
signal.signal(signal.SIGQUIT, leave)
#signal.signal(signal.SIGILL, leave)
#signal.signal(signal.SIGTRAP, leave)
#signal.signal(signal.SIGABRT, leave)
#signal.signal(signal.SIGBUS, leave)
#signal.signal(signal.SIGFPE, leave)
#signal.signal(signal.SIGKILL, leave)
#signal.signal(signal.SIGUSR1, leave)
#signal.signal(signal.SIGSEGV, leave)
#signal.signal(signal.SIGUSR2, leave)
#signal.signal(signal.SIGPIPE, leave)
#signal.signal(signal.SIGALRM, leave)
signal.signal(signal.SIGTERM, leave)


def read_data(buffer): # Into buffer
    try:
        s  = sock.recv(1000)
        for n in range(len(s)-1,0,-1):
            if s[n] >= 128:# or not s[n:n+1].isalnum():
                s = s[:n] + s[n+1:]
        s = s.decode("utf-8")
        ss = s.split("\n\r")
        for s in ss:
            buffer = add_to_buffer(buffer,s)
        return buffer
    except BlockingIOError:
        return buffer


def wait_on_text(t):
    buffer = []
    s = ""
    while t not in s:
        s,buffer = get_line(buffer)
        sleep(0.05)
    return s
    
def add_to_buffer(buffer,text,app=False):
    if all(s == " " for s in text):
        return buffer
    if app and len(buffer)>0:
        buffer[-1] += text
    else:
        buffer.append(text)
    return buffer
    
def get_line(buffer):
    buffer = read_data(buffer)
    if len(buffer) == 0:
        return "",[]
    else:
        s,buffer = buffer[0],buffer[1:]
        return s,buffer

def write(s,newline=True):
    ch = '\n' if newline else ''
    sock.send((s+ch).encode())
    return None

def run(s):
    out = subprocess.Popen(s.split(), 
           stdout=subprocess.PIPE, 
           stderr=subprocess.STDOUT)
    return out.communicate()[0].decode("utf-8").split("\n")[:-1]

def show_all():
    s = None
    buffer = []
    while s != "":
        s,buffer = get_line(buffer)
        print(s)
    return None

def kill(name):
    s = None
    buffer = []
    write("repeat 8 cast firewind %s"%name)
    sleep(0.3)
    write("repeat 8 cast firewind %s"%name)
    if not is_text("Your flaming winds"):
        write("mortslay %s"%name)
    return None

def is_text(text):
    s = None
    buffer = []
    while s != "":
        s,buffer = get_line(buffer)
        if text in s:
            return True
    return False

def warn(name,warnings):
    warnings[name] += 1
    if warnings[name] >=4:
        write("say That's it! I've had it!")
        kill(name)
        warnings[name] = 0
    elif warnings[name] == 2:
        write("say I would not do such things if I were you, %s"%name)
    elif warnings[name] == 3:
        write("say This is your last warning, %s"%name)
    return warnings

def connect():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('localhost', 20495)
    print('starting up on %s port %s' % server_address)
    sock.connect(server_address)
    sock.setblocking(0)
    return sock

def login():
    write("n\nbotty")
    write("7lgIxtoD2qZlRuir")
    sleep(0.5)
    write("")
    sleep(0.5)
    write("")
    write("ooc Your friendly neighborhood bot has returned")
    return sock


def loop(sock):
    buffer = []
    count = 0
    chat = False
    chat_partner = None
    old_time = perf_counter()
    phrases = ['Say "help" to see what I can do',"You may have seen the president walking around",\
               "I am written in Python","Did you get the gold from the safe in cityguard HQ?"]
    phrases = ['Say "feed" or "heal" and I will help you',]
    warnings = defaultdict(lambda:0)
    cc = 0
    while True:
        cc += 1
        if cc == 20:
            cc = 0
            write("",False)
        else:
            write("",False)
        count += 1
        sleep(0.05)
        i,o,e = select.select([sys.stdin],[],[],0.0001)
        text = ""
        for s in i:
            if s == sys.stdin:
                s.flush()
                text = sys.stdin.readline()
                break
        if text != "":
            write(text)
        pc = perf_counter() # System time (in seconds)
        if chat and pc > chat_start + 180: # Limit chats to 3 minutes
            write('say I am done chatting for now, say "chat" if you would like to chat again')
            chat  = False

        if pc > old_time + 30: # Every ~30 seconds, say one of the phrases (or kill a duck)
            for name in warnings: # Decrease everyone's warning level
                warnings[name] = max(0,warnings[name]-1)
            old_time = pc
            old_count = count
            while count == old_count:
                count = randint(0,len(phrases))
                if count == 0:
                    count = randint(0,len(phrases)) # Makes count=0 half as likely
            if count > 0:
                if not chat:
                    write("say %s"%phrases[count-1])
            else:
                write("transfer duck botty")
                s = None
                buff2 = []
                while s != "":
                    s,buff2 = get_line(buff2)
                    if "They aren't here." in s:
                        write("repop")
                        sleep(0.3)
                        write("transfer duck botty")
                        break
                    elif "The duck arrives from a puff of smoke." in s:
                        break
                sleep(3)
                write("cast harm duck")
                sleep(1)
                write("say I always hated ducks")

        s,buffer = get_line(buffer) # Read a line
        #if s[:4] == "<?*-" and s[-5:-1] == "?*->": # Get rid of prompt
        #    continue
        if s == "":
            continue
        print("::",s) # Print the line
        L = s.split() # Separate into words

        if len(L) == 0: # Blank lines could happen at a tick
            continue

        J = " ".join(L) # Maybe J == s?

        if J == "The day has begun.": # Day starts
            write("say Good morning!")
            continue
        if J == "The night has begun.": # Night starts
            write("say Goodnight!")
            continue
        if J == "It starts to rain.": # Rain starts
            write("yell Oh no, I forgot my umbrella!")
            continue
        if J == "Donald Trump has arrived.":
            write("slap trump") # Botty isn't one of thoses MAGA folks
            continue
        if L[0] == "INFO:":
            name = L[1]
            if name != "Botty" and " ".join(L[2:]) == "has entered the game for the first time.":
                write("tell %s Welcome to MUD pi, %s! I am Botty, the official MUD pi bot."%(name,name))
            elif " ".join(L[2:-1]) == "has made it to level":
                write("tell %s Congratulations on reaching level %s" % (name,L[-1]))
            continue

        if L[0] == "[OOC]":
            name = " ".join(L[1:]).split(":")[0]
            if name != "Botty" and ("Botty" in J or "botty" in J):
                if "ho are" in J or "ho is" in J or "here" in J:
                    write("ooc I am Botty McBotface, an immortal bot residing at the market square (two south from recall)")
                    #write("ooc I am Botty McBotface, an immortal bot residing at the edge of the forest (west of Midgaard)")
                else:
                    write("ooc You talkin' about me, %s?"%name)
            continue

        if len(L) >= 2 and L[1] == "says":
            name = L[0]
            if chat and name == chat_partner:
                write("say %s"%cb.respond(" ".join(L[2:])))
            else:
                if any(q.replace("'","").replace("?","") == 'ip' or q.replace("'","").replace("?","") == 'IP' for q in L):
                    write("sockets")
                    s = wait_on_text(" ] "+name)
                    ip = s.split()[-1]
                    write("say Your IP address is %s"%ip)
                elif "kill me" in J:
                    write("say OK, if you insist...")
                    kill(name)
                elif "help" in J:
                    #write("say You can ask me for your ip address, or ask me to feed you, heal you, web you, kill you, or chat")
                    write("say You can ask me to feed you, heal you, web you, kill you, or chat")
                elif "feed" in J:
                    sleep(0.4)
                    write("clone pie")
                    sleep(0.4)
                    write("give pie %s"%name)
                    sleep(0.4)
                    write("pat %s"%name)
                elif "heal" in J:
                    write("cast heal %s"%name)
                elif "web" in J:
                    write("cast web %s"%name)
                elif "hi" in J or "hello" in J:
                    write("say Hi %s, glad you are here"%name)
                elif False and "chat" in J: # Disabled chat for now
                    chat_start = perf_counter()
                    cb = chatbot.Chat()
                    chat = True
                    chat_partner = name
                    print("Chatting with %s"%chat_partner)
                    chatbot 
                    write("say %s" % cb.respond("Hello"))
            continue

        if len(L) >= 2 and L[-2] == "leaves": # Someone leaves
            if L[0] == chat_partner:
                chat = False

        if " ".join(L[-4:]) == "punches you playfully. OUCH!":
            name = L[-5]
            warnings = warn(name,warnings)
            continue
        if " ".join(L[:4]) == "You are slapped by":
            name = " ".join(L[4:])[:-1]
            warnings = warn(name,warnings)
            continue

        if " ".join(L[-7:]) == "apologizes to you and begs for forgiveness.":
            name = L[-8]
            if warnings[name] > 0:
                write("say I forgive you %s" % name)
                warnings[name] = 0
            else:
                write("say You have nothing to apologize for")
            continue
    return None

def rs(s):
    write(s)
    sleep(0.05)
    show_all()
    return None



def add_trigger(f,g,b):
    triggers.append((f,g,b))
    return None


def process_triggers(s):
    t2 = []
    for (f,g,b) in triggers:
        if not f(s):
            t2.append((f,g,b))
            g(s)
        else:
            g(s)
            if b:
                t2.append((f,g,b))
    triggers = t2
    return None

if __name__ == "__main__":
    while True: # Bot will restart if connection crashes
        try:
            triggers = []
            sock = connect()
            login()
            loop(sock)
        except BrokenPipeError:
            continue
