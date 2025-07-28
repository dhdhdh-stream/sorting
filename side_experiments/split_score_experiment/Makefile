CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Inn \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: clean_case

clean_case: ${OBJECTS} clean_case.o
	${CXX} ${CXXFLAGS} ${OBJECTS} clean_case.o -o clean_case ${LIBS}

-include ${DEPENDS} clean_case.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	clean_case.o clean_case.d clean_case
