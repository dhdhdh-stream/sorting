CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Imanual \
		   -Inn \
		   -Isolution \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find manual -name '*.cpp') \
		   $(shell find nn -name '*.cpp') \
		   $(shell find solution -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main viewer worker simple_init measure step_through suggestion run_manual

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

step_through: ${OBJECTS} step_through.o
	${CXX} ${CXXFLAGS} ${OBJECTS} step_through.o -o step_through ${LIBS}

suggestion: ${OBJECTS} suggestion.o
	${CXX} ${CXXFLAGS} ${OBJECTS} suggestion.o -o suggestion ${LIBS}

run_manual: ${OBJECTS} run_manual.o
	${CXX} ${CXXFLAGS} ${OBJECTS} run_manual.o -o run_manual ${LIBS}

-include ${DEPENDS} main.d viewer.d worker.d simple_init.d measure.d step_through.d suggestion.d run_manual.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	viewer.o viewer.d viewer \
	worker.o worker.d worker \
	simple_init.o simple_init.d simple_init \
	measure.o measure.d measure \
	step_through.o step_through.d step_through \
	suggestion.o suggestion.d suggestion \
	run_manual.o run_manual.d run_manual
