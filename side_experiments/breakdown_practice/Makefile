CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Ibreakdown \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find breakdown -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: split

split: ${OBJECTS} split.o
	${CXX} ${CXXFLAGS} ${OBJECTS} split.o -o split ${LIBS}

-include ${DEPENDS} split.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	split.o split.d split
