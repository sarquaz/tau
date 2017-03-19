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
	
SOURCES = $(wildcard src/*.c++ wildcard src/si/*.c++)
#SOURCES =  src/types.c++ src/si.++ src/log.c++ src/si
	
OBJECTS = $(SOURCES:.c++=.o)

#OBJECTS =  src/types.o src/si.o src/log.o

all: $(TARGET) 
	
clean: 
	find src -name *.o -o -name *.d | xargs rm -rf
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

test: rebuild
	cd test && $(MAKE); 
	
%.o: %.c++
	$(CXX) -c -o $@ $(TAU_CXXFLAGS) $(CPPDEPS) $<

	
.PHONY: all install test examples uninstall clean distclean  test
	
-include src/*.d