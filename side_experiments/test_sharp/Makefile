CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=1 \
		   -Iutils \
		   -Iworld_modeling
LIBS = -pthread
SOURCES := $(shell find utils -name '*.cpp') \
		   $(shell find world_modeling -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: test_sharp

test_sharp: ${OBJECTS} test_sharp.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_sharp.o -o test_sharp ${LIBS}

-include ${DEPENDS} test_sharp.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	test_sharp.o test_sharp.d test_sharp
