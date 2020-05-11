CC      = g++
AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CXXFLAGS += -std=c++17 -Wall -Wextra -Werror -fwrapv -march=native 
#override CXXFLAGS += -DUSE_SIMD

ifeq ($(OS),Windows_NT)
	override CXXFLAGS += -D_WIN32
	RM = del
else
	override LDFLAGS += -lX11 -pthread
	#RM = rm
endif

.PHONY : all debug libcubiomes clean

all: CXXFLAGS += -O3 -march=native
all: find_quadhuts find_compactbiomes clean

debug: CXXFLAGS += -DDEBUG -O0 -ggdb3
debug: find_quadhuts find_compactbiomes clean

libcubiomes: CXXFLAGS += -O3 -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

find_compactbiomes: find_compactbiomes.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_compactbiomes.o: find_compactbiomes.cpp
	$(CC) -c $(CXXFLAGS) $<

find_awesome_islands: find_awesome_islands.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_awesome_islands.o: find_awesome_islands.cpp
	$(CC) -c $(CXXFLAGS) $<

find_quadhuts: find_quadhuts.o layers.o generator.o finders.o 
	$(CC) -o $@ $^ $(LDFLAGS)

find_quadhuts.o: find_quadhuts.cpp
	$(CC) -c $(CXXFLAGS) $<

xmapview.o: xmapview.cpp xmapview.h
	$(CC) -c $(CXXFLAGS) $<

finders.o: finders.cpp finders.h
	$(CC) -c $(CXXFLAGS) $<

generator.o: generator.cpp generator.h
	$(CC) -c $(CXXFLAGS) $<

layers.o: layers.cpp layers.h
	$(CC) -c $(CXXFLAGS) $<

util.o: util.cpp util.h
	$(CC) -c $(CXXFLAGS) $<

clean:
	$(RM) *.o

