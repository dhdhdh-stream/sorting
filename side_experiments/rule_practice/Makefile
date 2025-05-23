CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
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

all: three_unroll one_unroll measure experiment viewer ideal first_variance

three_unroll: ${OBJECTS} three_unroll.o
	${CXX} ${CXXFLAGS} ${OBJECTS} three_unroll.o -o three_unroll ${LIBS}

one_unroll: ${OBJECTS} one_unroll.o
	${CXX} ${CXXFLAGS} ${OBJECTS} one_unroll.o -o one_unroll ${LIBS}

measure: ${OBJECTS} measure.o
	${CXX} ${CXXFLAGS} ${OBJECTS} measure.o -o measure ${LIBS}

experiment: ${OBJECTS} experiment.o
	${CXX} ${CXXFLAGS} ${OBJECTS} experiment.o -o experiment ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

ideal: ${OBJECTS} ideal.o
	${CXX} ${CXXFLAGS} ${OBJECTS} ideal.o -o ideal ${LIBS}

first_variance: ${OBJECTS} first_variance.o
	${CXX} ${CXXFLAGS} ${OBJECTS} first_variance.o -o first_variance ${LIBS}

-include ${DEPENDS} three_unroll.d one_unroll.d measure.d experiment.d viewer.d ideal.d first_variance.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	three_unroll.o three_unroll.d three_unroll \
	one_unroll.o one_unroll.d one_unroll \
	measure.o measure.d measure \
	experiment.o experiment.d experiment \
	viewer.o viewer.d viewer \
	ideal.o ideal.d ideal \
	first_variance.o first_variance.d first_variance
