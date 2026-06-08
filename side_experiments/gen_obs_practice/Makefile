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

all: split w_softmax

split: ${OBJECTS} split.o
	${CXX} ${CXXFLAGS} ${OBJECTS} split.o -o split ${LIBS}

w_softmax: ${OBJECTS} w_softmax.o
	${CXX} ${CXXFLAGS} ${OBJECTS} w_softmax.o -o w_softmax ${LIBS}

-include ${DEPENDS} split.d w_softmax.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	split.o split.d split \
	w_softmax.o w_softmax.d w_softmax
