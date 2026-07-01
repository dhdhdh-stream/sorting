CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: difficult no_additive deep normal_distribution_test deep_split

difficult: ${OBJECTS} difficult.o
	${CXX} ${CXXFLAGS} ${OBJECTS} difficult.o -o difficult ${LIBS}

no_additive: ${OBJECTS} no_additive.o
	${CXX} ${CXXFLAGS} ${OBJECTS} no_additive.o -o no_additive ${LIBS}

deep: ${OBJECTS} deep.o
	${CXX} ${CXXFLAGS} ${OBJECTS} deep.o -o deep ${LIBS}

normal_distribution_test: ${OBJECTS} normal_distribution_test.o
	${CXX} ${CXXFLAGS} ${OBJECTS} normal_distribution_test.o -o normal_distribution_test ${LIBS}

deep_split: ${OBJECTS} deep_split.o
	${CXX} ${CXXFLAGS} ${OBJECTS} deep_split.o -o deep_split ${LIBS}

-include ${DEPENDS} difficult.d no_additive.d deep.d normal_distribution_test.d deep_split.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	difficult.o difficult.d difficult \
	no_additive.o no_additive.d no_additive \
	deep.o deep.d deep \
	normal_distribution_test.o normal_distribution_test.d normal_distribution_test \
	deep_split.o deep_split.d deep_split
