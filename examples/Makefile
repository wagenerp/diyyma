
PREFIX=..
CC=gcc

ifeq ($(OS),Windows_NT)
	TARGETS=basic.exe screenquad.exe
	CFLAGS= -I"$(PREFIX)/include"
	LFLAGS= -L"$(PREFIX)/lib" \
	 	-ldiyyma -lSDL2 -lopenil -lopengl32 -lstdc++
else
	TARGETS=basic.sh screenquad.sh
	CFLAGS= -I"$(PREFIX)/include" -DGLEW_NO_GLU -DGLEW_STATIC -DDIYYMA_DEBUG \
			-std=c++11
	LFLAGS= -L"$(PREFIX)/lib" \
	 	-lSDL2 -lIL -lGL  -lm -lGLEW -lstdc++ \
	 	-ldiyyma
endif

all: $(TARGETS)

%.exe: %.cpp
	$(CC) $(CFLAGS) -o$@ $^ $(LFLAGS)

%.sh: %.cpp
	$(CC) $(CFLAGS) -o$@ $^ $(LFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o$@

.c.o:
	$(CC) $(CFLAGS) -c $< -o$@

ifeq ($(OS),Windows_NT)

clean:
	del $(subst /,\,$(TARGETS))
else

clean:
	rm $(TARGETS)
endif