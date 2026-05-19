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

all: normalize_state branch_input_change self_reference good_temporary extra_state extra_state_good_temporary

normalize_state: ${OBJECTS} normalize_state.o
	${CXX} ${CXXFLAGS} ${OBJECTS} normalize_state.o -o normalize_state ${LIBS}

branch_input_change: ${OBJECTS} branch_input_change.o
	${CXX} ${CXXFLAGS} ${OBJECTS} branch_input_change.o -o branch_input_change ${LIBS}

self_reference: ${OBJECTS} self_reference.o
	${CXX} ${CXXFLAGS} ${OBJECTS} self_reference.o -o self_reference ${LIBS}

good_temporary: ${OBJECTS} good_temporary.o
	${CXX} ${CXXFLAGS} ${OBJECTS} good_temporary.o -o good_temporary ${LIBS}

extra_state: ${OBJECTS} extra_state.o
	${CXX} ${CXXFLAGS} ${OBJECTS} extra_state.o -o extra_state ${LIBS}

extra_state_good_temporary: ${OBJECTS} extra_state_good_temporary.o
	${CXX} ${CXXFLAGS} ${OBJECTS} extra_state_good_temporary.o -o extra_state_good_temporary ${LIBS}

-include ${DEPENDS} normalize_state.d branch_input_change.d self_reference.d good_temporary.d extra_state.d extra_state_good_temporary.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	normalize_state.o normalize_state.d normalize_state \
	branch_input_change.o branch_input_change.d branch_input_change \
	self_reference.o self_reference.d self_reference \
	good_temporary.o good_temporary.d good_temporary \
	extra_state.o extra_state.d extra_state \
	extra_state_good_temporary.o extra_state_good_temporary.d extra_state_good_temporary
