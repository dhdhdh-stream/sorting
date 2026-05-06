CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Ibreakdown \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find breakdown -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: split multi_level_breakdown seq_shift

split: ${OBJECTS} split.o
	${CXX} ${CXXFLAGS} ${OBJECTS} split.o -o split ${LIBS}

multi_level_breakdown: ${OBJECTS} multi_level_breakdown.o
	${CXX} ${CXXFLAGS} ${OBJECTS} multi_level_breakdown.o -o multi_level_breakdown ${LIBS}

seq_shift: ${OBJECTS} seq_shift.o
	${CXX} ${CXXFLAGS} ${OBJECTS} seq_shift.o -o seq_shift ${LIBS}

-include ${DEPENDS} split.d multi_level_breakdown.d seq_shift.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	split.o split.d split \
	multi_level_breakdown.o multi_level_breakdown.d multi_level_breakdown \
	seq_shift.o seq_shift.d seq_shift
