CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Isum_tree \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find sum_tree -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: many_factors

many_factors: ${OBJECTS} many_factors.o
	${CXX} ${CXXFLAGS} ${OBJECTS} many_factors.o -o many_factors ${LIBS}

-include ${DEPENDS} many_factors.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	many_factors.o many_factors.d many_factors
