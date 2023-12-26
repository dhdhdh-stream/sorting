CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Itry_analyze \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find try_analyze -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: repeat sequence mix verify mix_sequence

repeat: ${OBJECTS} repeat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} repeat.o -o repeat ${LIBS}

sequence: ${OBJECTS} sequence.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sequence.o -o sequence ${LIBS}

mix: ${OBJECTS} mix.o
	${CXX} ${CXXFLAGS} ${OBJECTS} mix.o -o mix ${LIBS}

verify: ${OBJECTS} verify.o
	${CXX} ${CXXFLAGS} ${OBJECTS} verify.o -o verify ${LIBS}

mix_sequence: ${OBJECTS} mix_sequence.o
	${CXX} ${CXXFLAGS} ${OBJECTS} mix_sequence.o -o mix_sequence ${LIBS}

-include ${DEPENDS} repeat.d sequence.d mix.d verify.d mix_sequence.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	repeat.o repeat.d repeat \
	sequence.o sequence.d sequence \
	mix.o mix.d mix \
	verify.o verify.d verify \
	mix_sequence.o mix_sequence.d mix_sequence
