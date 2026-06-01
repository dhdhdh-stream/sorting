CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils \
		   -Iworld \
		   -Iworld_model
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp') \
		   $(shell find world_model -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main focus_sequence increasing_length

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

focus_sequence: ${OBJECTS} focus_sequence.o
	${CXX} ${CXXFLAGS} ${OBJECTS} focus_sequence.o -o focus_sequence ${LIBS}

increasing_length: ${OBJECTS} increasing_length.o
	${CXX} ${CXXFLAGS} ${OBJECTS} increasing_length.o -o increasing_length ${LIBS}

-include ${DEPENDS} main.d focus_sequence.d increasing_length.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	focus_sequence.o focus_sequence.d focus_sequence \
	increasing_length.o increasing_length.d increasing_length
