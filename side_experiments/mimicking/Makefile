CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   -Imimic \
		   -Inn \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find mimic -name '*.cpp') \
		   $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: capture_sample viewer train

capture_sample: ${OBJECTS} capture_sample.o
	${CXX} ${CXXFLAGS} ${OBJECTS} capture_sample.o -o capture_sample ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

train: ${OBJECTS} train.o
	${CXX} ${CXXFLAGS} ${OBJECTS} train.o -o train ${LIBS}

-include ${DEPENDS} capture_sample.d viewer.d train.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	capture_sample.o capture_sample.d capture_sample \
	viewer.o viewer.d viewer \
	train.o train.d train
