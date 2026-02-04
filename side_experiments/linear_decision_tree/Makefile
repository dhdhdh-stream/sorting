CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Idecision_tree \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find decision_tree -name '*.cpp') \
		   $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: many_factors simple xor

many_factors: ${OBJECTS} many_factors.o
	${CXX} ${CXXFLAGS} ${OBJECTS} many_factors.o -o many_factors ${LIBS}

simple: ${OBJECTS} simple.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple.o -o simple ${LIBS}

xor: ${OBJECTS} xor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} xor.o -o xor ${LIBS}

-include ${DEPENDS} many_factors.d simple.d xor.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	many_factors.o many_factors.d many_factors \
	simple.o simple.d simple \
	xor.o xor.d xor
