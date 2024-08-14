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

all: main focus viewer examine

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

focus: ${OBJECTS} focus.o
	${CXX} ${CXXFLAGS} ${OBJECTS} focus.o -o focus ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

examine: ${OBJECTS} examine.o
	${CXX} ${CXXFLAGS} ${OBJECTS} examine.o -o examine ${LIBS}

-include ${DEPENDS} main.d focus.d viewer.d examine.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	focus.o focus.d focus \
	viewer.o viewer.d viewer \
	examine.o examine.d examine
