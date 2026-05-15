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

all: no_self_dependency plasticity slow_no_self_dependency normalize_state direct low_iters build_samples

no_self_dependency: ${OBJECTS} no_self_dependency.o
	${CXX} ${CXXFLAGS} ${OBJECTS} no_self_dependency.o -o no_self_dependency ${LIBS}

plasticity: ${OBJECTS} plasticity.o
	${CXX} ${CXXFLAGS} ${OBJECTS} plasticity.o -o plasticity ${LIBS}

slow_no_self_dependency: ${OBJECTS} slow_no_self_dependency.o
	${CXX} ${CXXFLAGS} ${OBJECTS} slow_no_self_dependency.o -o slow_no_self_dependency ${LIBS}

normalize_state: ${OBJECTS} normalize_state.o
	${CXX} ${CXXFLAGS} ${OBJECTS} normalize_state.o -o normalize_state ${LIBS}

direct: ${OBJECTS} direct.o
	${CXX} ${CXXFLAGS} ${OBJECTS} direct.o -o direct ${LIBS}

low_iters: ${OBJECTS} low_iters.o
	${CXX} ${CXXFLAGS} ${OBJECTS} low_iters.o -o low_iters ${LIBS}

build_samples: ${OBJECTS} build_samples.o
	${CXX} ${CXXFLAGS} ${OBJECTS} build_samples.o -o build_samples ${LIBS}

-include ${DEPENDS} no_self_dependency.d plasticity.d slow_no_self_dependency.d normalize_state.d direct.d low_iters.d build_samples.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	no_self_dependency.o no_self_dependency.d no_self_dependency \
	plasticity.o plasticity.d plasticity \
	slow_no_self_dependency.o slow_no_self_dependency.d slow_no_self_dependency \
	normalize_state.o normalize_state.d normalize_state \
	direct.o direct.d direct \
	low_iters.o low_iters.d low_iters \
	build_samples.o build_samples.d build_samples
