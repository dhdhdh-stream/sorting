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

all: normalize_state branch_input_change self_reference good_temporary extra_state extra_state_good_temporary swap_factors examine_errors indirect direct_p2

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

swap_factors: ${OBJECTS} swap_factors.o
	${CXX} ${CXXFLAGS} ${OBJECTS} swap_factors.o -o swap_factors ${LIBS}

examine_errors: ${OBJECTS} examine_errors.o
	${CXX} ${CXXFLAGS} ${OBJECTS} examine_errors.o -o examine_errors ${LIBS}

indirect: ${OBJECTS} indirect.o
	${CXX} ${CXXFLAGS} ${OBJECTS} indirect.o -o indirect ${LIBS}

direct_p2: ${OBJECTS} direct_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} direct_p2.o -o direct_p2 ${LIBS}

-include ${DEPENDS} normalize_state.d branch_input_change.d self_reference.d good_temporary.d extra_state.d extra_state_good_temporary.d swap_factors.d examine_errors.d indirect.d direct_p2.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	normalize_state.o normalize_state.d normalize_state \
	branch_input_change.o branch_input_change.d branch_input_change \
	self_reference.o self_reference.d self_reference \
	good_temporary.o good_temporary.d good_temporary \
	extra_state.o extra_state.d extra_state \
	extra_state_good_temporary.o extra_state_good_temporary.d extra_state_good_temporary \
	swap_factors.o swap_factors.d swap_factors \
	examine_errors.o examine_errors.d examine_errors \
	indirect.o indirect.d indirect \
	direct_p2.o direct_p2.d direct_p2
