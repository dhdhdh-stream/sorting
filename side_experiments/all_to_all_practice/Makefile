CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: mult_scale simple_all_to_all

mult_scale: ${OBJECTS} mult_scale.o
	${CXX} ${CXXFLAGS} ${OBJECTS} mult_scale.o -o mult_scale ${LIBS}

simple_all_to_all: ${OBJECTS} simple_all_to_all.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_all_to_all.o -o simple_all_to_all ${LIBS}

-include ${DEPENDS} mult_scale.d simple_all_to_all.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	mult_scale.o mult_scale.d mult_scale \
	simple_all_to_all.o simple_all_to_all.d simple_all_to_all
