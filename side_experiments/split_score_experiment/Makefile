CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
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

all: clean_case measure test_alternate

clean_case: ${OBJECTS} clean_case.o
	${CXX} ${CXXFLAGS} ${OBJECTS} clean_case.o -o clean_case ${LIBS}

measure: ${OBJECTS} measure.o
	${CXX} ${CXXFLAGS} ${OBJECTS} measure.o -o measure ${LIBS}

test_alternate: ${OBJECTS} test_alternate.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_alternate.o -o test_alternate ${LIBS}

-include ${DEPENDS} clean_case.d measure.d test_alternate.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	clean_case.o clean_case.d clean_case \
	measure.o measure.d measure \
	test_alternate.o test_alternate.d test_alternate
