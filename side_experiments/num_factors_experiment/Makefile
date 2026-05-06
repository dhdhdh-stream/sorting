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

all: non_linear sum_then_analyze w_context

non_linear: ${OBJECTS} non_linear.o
	${CXX} ${CXXFLAGS} ${OBJECTS} non_linear.o -o non_linear ${LIBS}

sum_then_analyze: ${OBJECTS} sum_then_analyze.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sum_then_analyze.o -o sum_then_analyze ${LIBS}

w_context: ${OBJECTS} w_context.o
	${CXX} ${CXXFLAGS} ${OBJECTS} w_context.o -o w_context ${LIBS}

-include ${DEPENDS} non_linear.d sum_then_analyze.d w_context.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	non_linear.o non_linear.d non_linear \
	sum_then_analyze.o sum_then_analyze.d sum_then_analyze \
	w_context.o w_context.d w_context
