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

all: dual_error simple dual_error_v2

dual_error: ${OBJECTS} dual_error.o
	${CXX} ${CXXFLAGS} ${OBJECTS} dual_error.o -o dual_error ${LIBS}

simple: ${OBJECTS} simple.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple.o -o simple ${LIBS}

dual_error_v2: ${OBJECTS} dual_error_v2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} dual_error_v2.o -o dual_error_v2 ${LIBS}

-include ${DEPENDS} dual_error.d simple.d dual_error_v2.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	dual_error.o dual_error.d dual_error \
	simple.o simple.d simple \
	dual_error_v2.o dual_error_v2.d dual_error_v2
