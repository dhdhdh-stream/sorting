CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Isolution \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find solution -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: test_p1

test_p1: ${OBJECTS} test_p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_p1.o -o test_p1 ${LIBS}

-include ${DEPENDS} test_p1.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	test_p1.o test_p1.d test_p1
