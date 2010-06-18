CC      = gcc
PROF    = -DLINUX
NOCRYPT = 
C_FLAGS = -Wall -ggdb -O3 $(PROF) $(NOCRYPT)
L_FLAGS = -O2 $(PROF)

O_FILES = act_comm.o	act_info.o	act_move.o \
          act_obj.o	act_wiz.o	auction.o \
          ban.o		bank.o		bit.o \
          board.o	comm.o		const.o \
          clan.o	class.o		db.o \
          drunk2.o 	factions.o	fight.o \
	  handler.o 	helpolc.o	interp.o \
	  magic.o 	marry.o		mem.o \
          mprog_commands.o 		mprog_procs.o \
          mud_progs.o	olc.o           olc_act.o \
	  olc_save.o	newbits.o 	pty.o	\
      	  random.o	recycle.o       route_io.o \
    	  save.o	skills.o 	socialolc.o \
   	  ssm.o		string.o        todoolc.o \
  	  update.o	remort.o 

all: ember

ember: $(O_FILES)
	rm -f ember
	$(CC) $(L_FLAGS) -o ember $(O_FILES) -lm -lcrypt

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	-rm -f *.o *~ Ember.patch *.orig *.rej \#*

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
	@diff -c -P -N -x ember -x core -x Ember.patch -x ban.txt -x worktodo.txt -x help.txt ./original ./ > ./Ember.patch

update:
	rm -f orignial/*
	cp *.c *.h Makefile startup* original
