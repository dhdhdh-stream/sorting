CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Iutils \
		   -Iworld \
		   -Iworld_model
LIBS = -pthread
SOURCES := $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp') \
		   $(shell find world_model -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

-include ${DEPENDS} main.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main
