CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Itry_analyze \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find try_analyze -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: repeat sequence mix

repeat: ${OBJECTS} repeat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} repeat.o -o repeat ${LIBS}

sequence: ${OBJECTS} sequence.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sequence.o -o sequence ${LIBS}

mix: ${OBJECTS} mix.o
	${CXX} ${CXXFLAGS} ${OBJECTS} mix.o -o mix ${LIBS}

-include ${DEPENDS} repeat.d sequence.d mix.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	repeat.o repeat.d repeat \
	sequence.o sequence.d sequence \
	mix.o mix.d mix
