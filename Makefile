# gnuish Makefile

#OPTIONS = -g -Wall
OPTIONS = -O2 -Wall

SRCS =  deu.c gfx.c menus.c nodes.c textures.c edit.c levels.c mouse.c \
	objects.c things.c editobj.c names.c wads.c
	
OBJS = $(SRCS:.c=.o)


# Add dependencies of .h-files if you like ...

%.o : %.c
	gcc -c $(OPTIONS) $<


deu: $(OBJS)
	gcc $(OPTIONS) -o deu $(OBJS) -lm -lpc -lbcc -lgrx

deu.exe: deu
	strip deu
	coff2exe deu
        	