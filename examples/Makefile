CXX = g++
CPPFLAGS =  -I../include/tau  -std=c++11
LDFLAGS = -L../bin -ltau 

SOURCES = $(wildcard *.c++)
EXECUTABLES = $(SOURCES:.c++=)


all:  tau $(EXECUTABLES)

clean:
	rm -rf *.o
	rm -rf bin
	
	
%: %.c++
	 $(CXX) $(CPPFLAGS) -o bin/$@ $< $(LDFLAGS) 

tau:
	mkdir -p bin && cp ../bin/libtau.so bin  
         	

		
.PHONY: all clean 
	

