CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Inn \
		   -Isolution \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find solution -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main viewer worker simple_init measure combine examine_signal

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

worker: ${OBJECTS} worker.o
	${CXX} ${CXXFLAGS} ${OBJECTS} worker.o -o worker ${LIBS}

simple_init: ${OBJECTS} simple_init.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_init.o -o simple_init ${LIBS}

measure: ${OBJECTS} measure.o
	${CXX} ${CXXFLAGS} ${OBJECTS} measure.o -o measure ${LIBS}

combine: ${OBJECTS} combine.o
	${CXX} ${CXXFLAGS} ${OBJECTS} combine.o -o combine ${LIBS}

examine_signal: ${OBJECTS} examine_signal.o
	${CXX} ${CXXFLAGS} ${OBJECTS} examine_signal.o -o examine_signal ${LIBS}

-include ${DEPENDS} main.d viewer.d worker.d simple_init.d measure.d combine.d examine_signal.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	viewer.o viewer.d viewer \
	worker.o worker.d worker \
	simple_init.o simple_init.d simple_init \
	measure.o measure.d measure \
	combine.o combine.d combine \
	examine_signal.o examine_signal.d examine_signal
