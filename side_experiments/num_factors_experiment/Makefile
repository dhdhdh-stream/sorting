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

all: long_distance_xor long_distance_xor_self_ref double_xor big_xor difficult_self_ref

long_distance_xor: ${OBJECTS} long_distance_xor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} long_distance_xor.o -o long_distance_xor ${LIBS}

long_distance_xor_self_ref: ${OBJECTS} long_distance_xor_self_ref.o
	${CXX} ${CXXFLAGS} ${OBJECTS} long_distance_xor_self_ref.o -o long_distance_xor_self_ref ${LIBS}

double_xor: ${OBJECTS} double_xor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} double_xor.o -o double_xor ${LIBS}

big_xor: ${OBJECTS} big_xor.o
	${CXX} ${CXXFLAGS} ${OBJECTS} big_xor.o -o big_xor ${LIBS}

difficult_self_ref: ${OBJECTS} difficult_self_ref.o
	${CXX} ${CXXFLAGS} ${OBJECTS} difficult_self_ref.o -o difficult_self_ref ${LIBS}

-include ${DEPENDS} long_distance_xor.d long_distance_xor_self_ref.d double_xor.d big_xor.d difficult_self_ref.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	long_distance_xor.o long_distance_xor.d long_distance_xor \
	long_distance_xor_self_ref.o long_distance_xor_self_ref.d long_distance_xor_self_ref \
	double_xor.o double_xor.d double_xor \
	big_xor.o big_xor.d big_xor \
	difficult_self_ref.o difficult_self_ref.d difficult_self_ref
