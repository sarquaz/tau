CXX = g++

CPPFLAGS ?= -g -DLOG

UNAME := $(shell uname)

PLATFORM_LDFLAGS ?= 
PLATFORM_CPPFLAGS ?=

ifeq ($(UNAME), Linux)
    PLATFORM_LDFLAGS = -ldl -pthread
    PLATFORM_CPPFLAGS = -pthread
endif

include config.mk

TARGET = libtau.so

ifeq ($(BUILD),static)
TARGET = libtau.a
endif

AR = ar rcu
RANLIB= ranlib


CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
TAU_CXXFLAGS =  -fPIC -DPIC -std=c++0x    -Iinclude/tau $(PLATFORM_CPPFLAGS) $(CFLAGS) $(CPPFLAGS) 
	
#SOURCES = $(wildcard src/data.cpp src/types.cpp src/si.cpp src/log.cpp src/mem.cpp)
SOURCES =  src/types.c++ src/si.++ src/log.c++
	
#OBJECTS = $(SOURCES:.cpp=.o)

OBJECTS =  src/types.o src/si.o src/log.o

all: $(TARGET) 


clean_tau:
	rm -f src/*.o
	rm -f src/*.d
	rm -f bin
	
	-(cd test && $(MAKE) clean)	
	
clean: 
	rm -f src/*.o
	rm -f src/*.d
	rm -rf bin
	
	-(cd test && $(MAKE) clean)	


rebuild: clean
	make -j	
		

$(TARGET): $(OBJECTS)  
ifeq ($(BUILD),shared)
		mkdir -p bin && $(CXX) -shared  -fPIC -o bin/$@ $(OBJECTS)    $(PLATFORM_LDFLAGS) 
else
		$(AR) libtau.a $(OBJECTS)
		mkdir -p bin && $(RANLIB) bin/$@
endif



test: 
	cd test && $(MAKE); 
	
	 
%.o: %.c++
	$(CXX) -c -o $@ $(TAU_CXXFLAGS) $(CPPDEPS) $<

	
.PHONY: all install test examples uninstall clean distclean  test
	
-include src/*.d