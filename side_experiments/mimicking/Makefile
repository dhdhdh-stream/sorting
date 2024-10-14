CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   -Imimic \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find mimic -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: capture_sample viewer build_graph find_common_sequences break_by_location

capture_sample: ${OBJECTS} capture_sample.o
	${CXX} ${CXXFLAGS} ${OBJECTS} capture_sample.o -o capture_sample ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

build_graph: ${OBJECTS} build_graph.o
	${CXX} ${CXXFLAGS} ${OBJECTS} build_graph.o -o build_graph ${LIBS}

find_common_sequences: ${OBJECTS} find_common_sequences.o
	${CXX} ${CXXFLAGS} ${OBJECTS} find_common_sequences.o -o find_common_sequences ${LIBS}

break_by_location: ${OBJECTS} break_by_location.o
	${CXX} ${CXXFLAGS} ${OBJECTS} break_by_location.o -o break_by_location ${LIBS}

-include ${DEPENDS} capture_sample.d viewer.d build_graph.d find_common_sequences.d break_by_location.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	capture_sample.o capture_sample.d capture_sample \
	viewer.o viewer.d viewer \
	build_graph.o build_graph.d build_graph \
	find_common_sequences.o find_common_sequences.d find_common_sequences \
	break_by_location.o break_by_location.d break_by_location
