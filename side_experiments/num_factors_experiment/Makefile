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

all: magnify_diff md_two_dimension_one_factor md_two_dimension_two_factor increment increment_one_factor increment_3d increment_3d_2f

magnify_diff: ${OBJECTS} magnify_diff.o
	${CXX} ${CXXFLAGS} ${OBJECTS} magnify_diff.o -o magnify_diff ${LIBS}

md_two_dimension_one_factor: ${OBJECTS} md_two_dimension_one_factor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} md_two_dimension_one_factor.o -o md_two_dimension_one_factor ${LIBS}

md_two_dimension_two_factor: ${OBJECTS} md_two_dimension_two_factor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} md_two_dimension_two_factor.o -o md_two_dimension_two_factor ${LIBS}

increment: ${OBJECTS} increment.o
	${CXX} ${CXXFLAGS} ${OBJECTS} increment.o -o increment ${LIBS}

increment_one_factor: ${OBJECTS} increment_one_factor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} increment_one_factor.o -o increment_one_factor ${LIBS}

increment_3d: ${OBJECTS} increment_3d.o
	${CXX} ${CXXFLAGS} ${OBJECTS} increment_3d.o -o increment_3d ${LIBS}

increment_3d_2f: ${OBJECTS} increment_3d_2f.o
	${CXX} ${CXXFLAGS} ${OBJECTS} increment_3d_2f.o -o increment_3d_2f ${LIBS}

-include ${DEPENDS} magnify_diff.d md_two_dimension_one_factor.d md_two_dimension_two_factor.d increment.d increment_one_factor.d increment_3d.d increment_3d_2f.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	magnify_diff.o magnify_diff.d magnify_diff \
	md_two_dimension_one_factor.o md_two_dimension_one_factor.d md_two_dimension_one_factor \
	md_two_dimension_two_factor.o md_two_dimension_two_factor.d md_two_dimension_two_factor \
	increment.o increment.d increment \
	increment_one_factor.o increment_one_factor.d increment_one_factor \
	increment_3d.o increment_3d.d increment_3d \
	increment_3d_2f.o increment_3d_2f.d increment_3d_2f
