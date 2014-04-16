
PREFIX=..

TARGETS=basic.exe screenquad.exe

CC=gcc

CFLAGS= -I"$(PREFIX)/include"
LFLAGS= -L"$(PREFIX)/lib" \
	 -ldiyyma -lSDL2 -lopenil -lopengl32 -lstdc++


lst:
	echo $(OBJECTS)

all: $(TARGETS)

%.exe: %.cpp
	$(CC) $(CFLAGS) -o$@ $^ $(LFLAGS)


.cpp.o:
	$(CC) $(CFLAGS) -c $< -o$@

.c.o:
	$(CC) $(CFLAGS) -c $< -o$@



clean:
	del $(subst /,\,$(TARGETS) $(OBJECTS))

