CC      = gcc
PROF    = -DLINUX 
NOCRYPT =
C_FLAGS = -Wall -ggdb -O3 $(PROF) $(NOCRYPT)
L_FLAGS = -O2 $(PROF) -lcrypt

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o save.o skills.o special.o \
	  update.o bit.o string.o mem.o olc.o olc_act.o olc_save.o \
	  mob_commands.o mob_prog.o board.o random.o clan.o

all: ember

ember: $(O_FILES)
	rm -f ember
	$(CC) $(L_FLAGS) -o ember $(O_FILES) -lm

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	-rm -f *.o *~ Ember.patch *.orig *.rej

distclean:
	-rm -f ../area/*~ ../area/*.txt
	-rm -f ../src/*~ ../*~
	-rm -f ../gods/* ../log/* ../player/*

src:
	-rm -f src.tar.gz
	-tar -czvf src.tar.gz Makefile *.c *.h

diff: clean
	@clear
	@echo Diffing against ./original directory and creating Ember.patch.
	@echo Ignore the following error.
	@echo  
	@diff -c -P -N -x ember -x core -x Ember.patch ./original ./ > ./Ember.patch

update:
	rm -f orignial/*
	cp *.c *.h Makefile startup* original
