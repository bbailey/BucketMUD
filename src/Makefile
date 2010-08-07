CC      = gcc
PROF    = -DLINUX
NOCRYPT = 
OPT     = # -O2
C_FLAGS = -Wall -ggdb $(OPT) $(PROF) $(NOCRYPT) `pkg-config --cflags glib-2.0`
L_FLAGS = $(OPT) $(PROF) -lm -lcrypt `pkg-config --libs glib-2.0`

O_FILES = act_comm.o	act_info.o	act_move.o \
          act_obj.o	act_wiz.o	auction.o \
          ban.o		bank.o		bit.o 		BitVector.o     BitVectorStr.o\
          board.o	bv_tables.o comm.o		const.o \
          clan.o	class.o		db.o \
          drunk2.o 	factions.o	fight.o \
	  handler.o 	helpolc.o	interp.o \
	  magic.o 	marry.o		mem.o \
          mprog_commands.o 		mprog_procs.o \
          mud_progs.o	olc.o           olc_act.o \
	  olc_save.o	newbits.o \
      	  random.o	recycle.o \
    	  save.o	skills.o 	socialolc.o \
   	  ssm.o		string.o \
  	  update.o	remort.o 

## IMC2 support.
IMC     = 1

ifdef IMC
   O_FILES := imc.o sha256.o $(O_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCROM -DIMCEMBER
endif

all: ember

ember: $(O_FILES)
	rm -f ember
	$(CC) -o ember $(O_FILES) $(L_FLAGS)

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

ifdef IMC
splint:
	splint -showcolumn $(PROF) `pkg-config --cflags glib-2.0` -DIMC -DIMCROM -DIMCEMBER *.[ch]
else
splint:
	splint -showcolumn $(PROF) `pkg-config --cflags glib-2.0` *.c
endif
