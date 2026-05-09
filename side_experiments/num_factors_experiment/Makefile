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

all: multi_stable_step multi_wo_final multi_stable_2d difficult

multi_stable_step: ${OBJECTS} multi_stable_step.o
	${CXX} ${CXXFLAGS} ${OBJECTS} multi_stable_step.o -o multi_stable_step ${LIBS}

multi_wo_final: ${OBJECTS} multi_wo_final.o
	${CXX} ${CXXFLAGS} ${OBJECTS} multi_wo_final.o -o multi_wo_final ${LIBS}

multi_stable_2d: ${OBJECTS} multi_stable_2d.o
	${CXX} ${CXXFLAGS} ${OBJECTS} multi_stable_2d.o -o multi_stable_2d ${LIBS}

difficult: ${OBJECTS} difficult.o
	${CXX} ${CXXFLAGS} ${OBJECTS} difficult.o -o difficult ${LIBS}

-include ${DEPENDS} multi_stable_step.d multi_wo_final.d multi_stable_2d.d difficult.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	multi_stable_step.o multi_stable_step.d multi_stable_step \
	multi_wo_final.o multi_wo_final.d multi_wo_final \
	multi_stable_2d.o multi_stable_2d.d multi_stable_2d \
	difficult.o difficult.d difficult
