include ../config.mk
CXX = g++
CPPFLAGS =  -I../include/tau  -g -DLOG -std=c++0x 
LDFLAGS = -L../bin -ltau 

SOURCES = $(wildcard *.c++)
#EXECUTABLES = $(SOURCES:.c++)

EXECUTABLES = Map

all:  clean $(EXECUTABLES)  run

tau:
	cd .. && $(MAKE)
clean:
	rm -rf *.o
	rm -rf bin
	
	
%: %.c++
	$(CXX) $(CPPFLAGS) -o $@   $< $(LDFLAGS)  
 	
run:
	 mkdir bin && cp $(EXECUTABLES) bin && cd .. && test/run.sh	

		
.PHONY: all clean 
	
-include *.d	

