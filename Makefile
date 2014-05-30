# Makefile for building using MinGW

TARGETS=lib/libdiyyma.a

OBJECTS=\
	$(subst \,/,\
		$(subst .cpp,.o, $(shell dir /B /S src\*.cpp)) \
		$(subst .c,.o, $(shell dir /B /S src\*.c)))

CC=gcc
AR=ar


CFLAGS= -I"include" -DGLEW_NO_GLU -DGLEW_STATIC -DDIYYMA_DEBUG
CPPFLAGS= -I"include" -DGLEW_NO_GLU -DGLEW_STATIC -DDIYYMA_DEBUG -std=c++11

all: $(TARGETS)


lib/libdiyyma.a: $(OBJECTS)
	$(AR) cr $@ $^ 

.cpp.o:
	$(CC) $(CPPFLAGS) -c $< -o$@

.c.o:
	$(CC) $(CFLAGS) -c $< -o$@


clean:
	del $(subst /,\,$(TARGETS) $(OBJECTS))

