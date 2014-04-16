# Makefile for building using MinGW

TARGETS=lib/libdiyyma.a

OBJECTS=\
	$(subst \,/,\
		$(subst .cpp,.o, $(shell dir /B /S src\*.cpp)) \
		$(subst .c,.o, $(shell dir /B /S src\*.c)))

CC=gcc
AR=ar

CFLAGS= -I"include" -DIL_STATIC_LIB -DGLEW_NO_GLU -DGLEW_STATIC

all: $(TARGETS)


lib/libdiyyma.a: $(OBJECTS)
	$(AR) cr $@ $^ 

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o$@

.c.o:
	$(CC) $(CFLAGS) -c $< -o$@


clean:
	del $(subst /,\,$(TARGETS) $(OBJECTS))

