include ../config.mk
CXX = g++
CPPFLAGS =  -I../include/tau  -g -DLOG -std=c++0x 
LDFLAGS = -L../ -ltau 

SOURCES = $(wildcard *.c++)
#EXECUTABLES = $(SOURCES:.c++)

EXECUTABLES = Map

#all:  clean $(EXECUTABLES)  run
all: clean $(EXECUTABLES)

tau:
	cd .. && $(MAKE)
clean:
	rm -rf *.o
	rm -rf $(EXECUTABLES)
	
	
%: %.c++
	$(CXX) $(CPPFLAGS) -o $@   $< $(LDFLAGS)  
 	
run:
	cd .. && test/run.sh	
		
.PHONY: all clean 
	
-include *.d	

