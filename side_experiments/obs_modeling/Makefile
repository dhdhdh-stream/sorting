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

all: simple_sort

simple_sort: ${OBJECTS} simple_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_sort.o -o simple_sort ${LIBS}

-include ${DEPENDS} simple_sort.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	simple_sort.o simple_sort.d simple_sort
