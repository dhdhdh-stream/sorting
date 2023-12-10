CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Iutils \
		   -Iworld_modeling
LIBS = -pthread
SOURCES := $(shell find utils -name '*.cpp') \
		   $(shell find world_modeling -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: repeat sequence

repeat: ${OBJECTS} repeat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} repeat.o -o repeat ${LIBS}

sequence: ${OBJECTS} sequence.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sequence.o -o sequence ${LIBS}

-include ${DEPENDS} repeat.d sequence.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	repeat.o repeat.d repeat \
	sequence.o sequence.d sequence
