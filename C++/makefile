BIN=bin\Particles.exe

CC=g++
SHELL=/bin/sh

CPPFLAGS=\
-Ofast	\
-std=c++11	\
-LC:\MinGW\lib	\
-IC:\MinGW\include

LIBS =\
-lsfml-graphics	\
-lsfml-window	\
-lsfml-system	\
-lsfgui

SRCS=\
main.cpp	\
particles.cpp	\
ball.cpp	\
quad.cpp	\
blackHole.cpp

HDRS=\
ball.hpp	\
barrier.hpp	\
blackHole.hpp	\
input.hpp	\
particles.hpp	\
quad.hpp	\
simulation.hpp	\
spinlock.hpp

OBJS=\
main.o	\
particles.o	\
ball.o	\
quad.o	\
blackHole.o

$(BIN): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) $(LIBS) -o $(BIN)

srcs:	$(HDRS)  $(SRCS) 
	echo $(HDRS)  $(SRCS) 

all:
	touch $(HDRS)  $(SRCS) 

clean:
	/bin/rm -f *.o $(BIN)*.tar *~ core a.out

tar: makefile $(SRCS) $(HDRS)
	tar -cvf $(BIN).tar makefile $(SRCS) $(HDRS) 
	ls -l $(BIN)*tar