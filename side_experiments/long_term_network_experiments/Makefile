CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: sum_difference simple_noise limited_data combine_data

sum_difference: ${OBJECTS} sum_difference.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sum_difference.o -o sum_difference ${LIBS}

simple_noise: ${OBJECTS} simple_noise.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_noise.o -o simple_noise ${LIBS}

limited_data: ${OBJECTS} limited_data.o
	${CXX} ${CXXFLAGS} ${OBJECTS} limited_data.o -o limited_data ${LIBS}

combine_data: ${OBJECTS} combine_data.o
	${CXX} ${CXXFLAGS} ${OBJECTS} combine_data.o -o combine_data ${LIBS}

-include ${DEPENDS} sum_difference.d simple_noise.d limited_data.d combine_data.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	sum_difference.o sum_difference.d sum_difference \
	simple_noise.o simple_noise.d simple_noise \
	limited_data.o limited_data.d limited_data \
	combine_data.o combine_data.d combine_data
