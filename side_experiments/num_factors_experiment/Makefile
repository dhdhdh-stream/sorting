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

all: phase_out phase_out_add_factor phase_out_p2 low_sample

phase_out: ${OBJECTS} phase_out.o
	${CXX} ${CXXFLAGS} ${OBJECTS} phase_out.o -o phase_out ${LIBS}

phase_out_add_factor: ${OBJECTS} phase_out_add_factor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} phase_out_add_factor.o -o phase_out_add_factor ${LIBS}

phase_out_p2: ${OBJECTS} phase_out_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} phase_out_p2.o -o phase_out_p2 ${LIBS}

low_sample: ${OBJECTS} low_sample.o
	${CXX} ${CXXFLAGS} ${OBJECTS} low_sample.o -o low_sample ${LIBS}

-include ${DEPENDS} phase_out.d phase_out_add_factor.d phase_out_p2.d low_sample.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	phase_out.o phase_out.d phase_out \
	phase_out_add_factor.o phase_out_add_factor.d phase_out_add_factor \
	phase_out_p2.o phase_out_p2.d phase_out_p2 \
	low_sample.o low_sample.d low_sample
