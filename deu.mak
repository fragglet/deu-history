#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCCDOS  = Bcc +BccDos.cfg 
TLINK   = TLink
TLIB    = TLib
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGSDOS =  -LD:\BC4\LIB
IDE_BFLAGS = 
LLATDOS_deudexe =  -c -Tde
RLATDOS_deudexe = 
BLATDOS_deudexe = 
LEAT_deudexe = $(LLATDOS_deudexe)
REAT_deudexe = $(RLATDOS_deudexe)
BEAT_deudexe = $(BLATDOS_deudexe)

#
# Dependency List
#
Dep_deu = \
   deu.exe

deu : BccDos.cfg $(Dep_deu)
  echo MakeNode deu

Dep_deudexe = \
   deu.obj\
   edit.obj\
   editobj.obj\
   gfx.obj\
   levels.obj\
   memory.obj\
   menus.obj\
   mouse.obj\
   names.obj\
   nodes.obj\
   objects.obj\
   swapmem.obj\
   textures.obj\
   things.obj\
   wads.obj

deu.exe : $(Dep_deudexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGSDOS) $(LEAT_deudexe) +
D:\BC4\LIB\c0h.obj+
/o+ deu.obj+
edit.obj+
editobj.obj+
/o- gfx.obj+
/o+ levels.obj+
/o- memory.obj+
/o+ menus.obj+
/o- mouse.obj+
/o+ names.obj+
nodes.obj+
objects.obj+
/o- swapmem.obj+
/o+ textures.obj+
things.obj+
wads.obj
$<,$*
D:\BC4\LIB\overlay.lib+
D:\BC4\LIB\graphics.lib+
D:\BC4\LIB\emu.lib+
D:\BC4\LIB\mathh.lib+
D:\BC4\LIB\ch.lib

|

deu.obj :  deu.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ deu.c

edit.obj :  edit.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ edit.c

editobj.obj :  editobj.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ editobj.c

gfx.obj :  gfx.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ gfx.c

levels.obj :  levels.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ levels.c

memory.obj :  memory.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ memory.c

menus.obj :  menus.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ menus.c

mouse.obj :  mouse.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ mouse.c

names.obj :  names.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ names.c

nodes.obj :  nodes.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ nodes.c

objects.obj :  objects.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ objects.c

swapmem.obj :  swapmem.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ swapmem.c

textures.obj :  textures.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ textures.c

things.obj :  things.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ things.c

wads.obj :  wads.c
  $(BCCDOS) -P- -c $(CEAT_deudexe) -o$@ wads.c

# Compiler configuration file
BccDos.cfg : 
   Copy &&|
-W-
-R
-v
-vi
-X-
-H
-ID:\BC4\INCLUDE
-H=deu.csm
-DCIRRUS_PATCH
-X-
-v
-3
-Og
-Os
-O-m
-Op
-Oi
-O-v
-Z
-O
-Oe
-Ol
-Ob
-OW
-Ff
-dc-
-h-
-ff-
-f
-Y
| $@


