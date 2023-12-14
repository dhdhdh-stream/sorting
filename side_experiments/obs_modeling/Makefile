CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Inn \
		   -Iutils \
		   -Iworld_modeling
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world_modeling -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: repeat

repeat: ${OBJECTS} repeat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} repeat.o -o repeat ${LIBS}

-include ${DEPENDS} repeat.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	repeat.o repeat.d repeat
