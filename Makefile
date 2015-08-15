# Makefile for building using MinGW

TARGETS=lib/libdiyyma.a

CC=gcc
AR=ar

CFLAGS= -I"include" -DGLEW_NO_GLU -DGLEW_STATIC -DDIYYMA_DEBUG
CPPFLAGS= -I"include" -DGLEW_NO_GLU -DGLEW_STATIC -DDIYYMA_DEBUG -std=c++11

ifeq ($(OS),Windows_NT)

	OBJECTS=\
		$(subst \,/,\
			$(subst .cpp,.o, $(shell dir /B /S src\*.cpp)) \
			$(subst .c,.o, $(shell dir /B /S src\*.c)))

all: $(TARGETS)

lib/libdiyyma.a: $(OBJECTS)
	$(AR) cr $@ $^

.cpp.o:
	$(CC) $(CPPFLAGS) -c $< -o$@

.c.o:
	$(CC) $(CFLAGS) -c $< -o$@

clean:
	del $(subst /,\,$(TARGETS) $(OBJECTS))

else

	CPP_FILES=$(shell find src -name *.cpp)
	C_FILES=$(shell find src -name *.c)
	OBJECTS=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.cpp.o))) \
			$(addprefix obj/,$(notdir $(C_FILES:.c=.c.o)))

	VPATH=$(shell find src -type d)


all: MAKE_OBJECT_DIR $(TARGETS)

lib/libdiyyma.a: $(OBJECTS)
	$(AR) cr $@ $^

MAKE_OBJECT_DIR:
	mkdir -p obj
	mkdir -p lib

obj/%.cpp.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o$@

obj/%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o$@

clean:
	rm -rf obj/*.o
	rm $(TARGETS)

endif
