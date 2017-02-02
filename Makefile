#OPTFLAGS=-O3 -fomit-frame-pointer -funroll-loops
OPTFLAGS=-O3 -fomit-frame-pointer -funroll-loops -march=native
#OPTFLAGS=-xM -O3

CC=gcc -m32
#CC=icc
CXX=g++ -m32
#CXX=icpc
CFLAGS=-pthread -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/libdrm -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/freetype2 -I/usr/include/libpng16 $(shell sdl-config --cflags) -Imast -Idoze -D__cdecl=

#CFLAGS= $(shell pkg-config gtk+-2.0  --cflags) $(OPTFLAGS) $(shell sdl-config --cflags) -Imast -Idoze -D__cdecl=
CXXFLAGS= $(CFLAGS) -fno-exceptions

DOZEOBJ = doze/doze.o doze/dozea.o
DAMOBJ = doze/dam.o doze/dama.o doze/damc.o doze/dame.o doze/damf.o doze/damj.o doze/damm.o doze/damo.o doze/damt.o
MASTOBJ = mast/area.o mast/dpsg.o mast/draw.o mast/emu2413.o mast/frame.o mast/load.o mast/map.o mast/mast.o mast/mem.o mast/samp.o mast/snd.o mast/vgm.o
SDLOBJ = sdl/main.o

all: dega

dega: $(SDLOBJ) $(DOZEOBJ) $(MASTOBJ)
	$(CC) -o degaJoe sdl/main.o $(DOZEOBJ) $(MASTOBJ) $(shell sdl-config --libs) $(shell pkg-config gtk+-2.0  --libs) /lib/libm.so.6

doze/dozea.o: doze/dozea.asm
	nasm -f elf doze/dozea.asm

doze/dozea.asm: doze/dam
	cd doze ; ./dam
	sed -f doze/doze.cmd.sed <doze/dozea.asm >doze/dozea.asm.new
	mv doze/dozea.asm.new doze/dozea.asm

doze/dam: $(DAMOBJ)
	$(CC) -o doze/dam $(DAMOBJ)

clean:
	rm -f $(DOZEOBJ) $(DAMOBJ) $(MASTOBJ) $(SDLOBJ) doze/dozea.asm* doze/dam dega

distclean: clean
	rm -f *~ */*~
