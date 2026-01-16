CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: signal_better bad_signal incomplete_signal limit_input

signal_better: ${OBJECTS} signal_better.o
	${CXX} ${CXXFLAGS} ${OBJECTS} signal_better.o -o signal_better ${LIBS}

bad_signal: ${OBJECTS} bad_signal.o
	${CXX} ${CXXFLAGS} ${OBJECTS} bad_signal.o -o bad_signal ${LIBS}

incomplete_signal: ${OBJECTS} incomplete_signal.o
	${CXX} ${CXXFLAGS} ${OBJECTS} incomplete_signal.o -o incomplete_signal ${LIBS}

limit_input: ${OBJECTS} limit_input.o
	${CXX} ${CXXFLAGS} ${OBJECTS} limit_input.o -o limit_input ${LIBS}

-include ${DEPENDS} signal_better.d bad_signal.d incomplete_signal.d limit_input.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	signal_better.o signal_better.d signal_better \
	bad_signal.o bad_signal.d bad_signal \
	incomplete_signal.o incomplete_signal.d incomplete_signal \
	limit_input.o limit_input.d limit_input
