# gnuish Makefile
 
OPTIONS = -O2 -g -pedantic -Wall
CC = gcc $(OPTIONS)

SRCS =  deu.c gfx.c menus.c nodes.c textures.c edit.c levels.c mouse.c \
	objects.c things2.c editobj.c names.c wads.c readcfg.c
	
OBJS = $(SRCS:.c=.o)

deth.exe: deth
	strip deth
	coff2exe deth
        	
deth: $(OBJS)
	gcc $(OPTIONS) -o deth $(OBJS) -lm -lpc -lbcc -lgrx

debug: deth

clean:
	del *.o
	del deth
	del deth.exe

zip:
	zip -r -u source.zip *.c *.h makefile *.cfg *.dm? *.her *.dm

tags: dummy
	ctags -r -v *.c *.h

dummy:

# dependencies produced by 'gcc -MM' 

deu.o : deu.c deu.h deu-go32.h 
edit.o : edit.c deu.h deu-go32.h levels.h wstructs.h
editobj.o : editobj.c deu.h deu-go32.h levels.h wstructs.h
gfx.o : gfx.c deu.h deu-go32.h 
levels.o : levels.c deu.h deu-go32.h wstructs.h
menus.o : menus.c deu.h deu-go32.h 
mouse.o : mouse.c deu.h deu-go32.h 
names.o : names.c deu.h deu-go32.h 
nodes.o : nodes.c deu.h deu-go32.h levels.h wstructs.h
objects.o : objects.c deu.h deu-go32.h levels.h wstructs.h
textures.o : textures.c deu.h deu-go32.h 
things2.o : things2.c deu.h deu-go32.h 
wads.o : wads.c deu.h deu-go32.h 
readcfg.o: readcfg.c deu.h deu-go32.h wstructs.h
