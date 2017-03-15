CXX = g++
CPPFLAGS =  -I../include/tau  -g -DLOG -std=c++0x 
LDFLAGS = -L../bin -ltau 

SOURCES = $(wildcard *.c++)
#EXECUTABLES = $(SOURCES:.c++)

EXECUTABLES = Mem

all:  tau $(EXECUTABLES)  #run

clean:
	rm -rf *.o
	rm -rf bin
	
	
%: %.c++
	 $(CXX) $(CPPFLAGS) -o bin/$@ $< $(LDFLAGS) 

tau:
	mkdir -p bin && cp ../bin/libtau.so bin  
         	
run:

	cp ../bin/libtau.so bin
#
#              # && cd .. && test/run.sh

		
.PHONY: all clean 
	

