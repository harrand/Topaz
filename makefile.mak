# Topaz Makefile
CC=gcc
CPP=g++

C_CPLFLAGS=-c -w
CPP_CPLFLAGS=-std=c++14 -c

CPLDIR=./cpl
INCDIR=./inc
LIBDIR=./lib
RESDIR=./res
C_SRCS=$(wildcard src/*.c)
CPP_SRCS=$(wildcard src/*.cpp)

TEST_RES=./res/exe/topaz_test.res

LIBS=-lOpenGL32 -lSDL2 -lSDL2main -lmdl
LNKFLAGS=-std=c++14 -static-libgcc -static-libstdc++ -Wall -pedantic -O3

C_OBJECTS=$(patsubst %.c,%.o,$(C_SRCS))
CPP_OBJECTS=$(patsubst %.cpp,%.o,$(CPP_SRCS))

CPP_OBJECTS += $(C_OBJECTS)

begin: cplc cplcpp lnk

cplc: $(C_SRCS)
	$(CC) $(C_CPLFLAGS) $(C_SRCS) -I $(INCDIR)
	#	gcc -c -w src/*.c -I ./inc

cplcpp: $(CPP_SRCS)
	$(CPP) $(CPP_CPLFLAGS) $(C_SRCS) -I $(INCDIR)
	# g++ -stdc++14 -c src/*.cpp -I ./inc
	
lnk: $(CPP_OBJECTS)
	$(CPP) $(LNKFLAGS) $(C_SRCS) -L$(LIBDIR) $(LIBS) $(TEST_RES) -o "topaz-test.exe"
