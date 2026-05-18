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

all: normalize_state branch_input_change

normalize_state: ${OBJECTS} normalize_state.o
	${CXX} ${CXXFLAGS} ${OBJECTS} normalize_state.o -o normalize_state ${LIBS}

branch_input_change: ${OBJECTS} branch_input_change.o
	${CXX} ${CXXFLAGS} ${OBJECTS} branch_input_change.o -o branch_input_change ${LIBS}

-include ${DEPENDS} normalize_state.d branch_input_change.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	normalize_state.o normalize_state.d normalize_state \
	branch_input_change.o branch_input_change.d branch_input_change
