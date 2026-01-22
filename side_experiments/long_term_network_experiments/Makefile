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

all: sum_difference simple_noise limited_data combine_data five_percent fast_to_slow ten_percent

sum_difference: ${OBJECTS} sum_difference.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sum_difference.o -o sum_difference ${LIBS}

simple_noise: ${OBJECTS} simple_noise.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_noise.o -o simple_noise ${LIBS}

limited_data: ${OBJECTS} limited_data.o
	${CXX} ${CXXFLAGS} ${OBJECTS} limited_data.o -o limited_data ${LIBS}

combine_data: ${OBJECTS} combine_data.o
	${CXX} ${CXXFLAGS} ${OBJECTS} combine_data.o -o combine_data ${LIBS}

five_percent: ${OBJECTS} five_percent.o
	${CXX} ${CXXFLAGS} ${OBJECTS} five_percent.o -o five_percent ${LIBS}

fast_to_slow: ${OBJECTS} fast_to_slow.o
	${CXX} ${CXXFLAGS} ${OBJECTS} fast_to_slow.o -o fast_to_slow ${LIBS}

ten_percent: ${OBJECTS} ten_percent.o
	${CXX} ${CXXFLAGS} ${OBJECTS} ten_percent.o -o ten_percent ${LIBS}

-include ${DEPENDS} sum_difference.d simple_noise.d limited_data.d combine_data.d five_percent.d fast_to_slow.d ten_percent.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	sum_difference.o sum_difference.d sum_difference \
	simple_noise.o simple_noise.d simple_noise \
	limited_data.o limited_data.d limited_data \
	combine_data.o combine_data.d combine_data \
	five_percent.o five_percent.d five_percent \
	fast_to_slow.o fast_to_slow.d fast_to_slow \
	ten_percent.o ten_percent.d ten_percent
