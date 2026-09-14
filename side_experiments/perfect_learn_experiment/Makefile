CXX = g++
# CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
# 		   `pkg-config --cflags eigen3` \
# 		   -DMDEBUG=1 \
# 		   -Inn \
# 		   -Isolution \
# 		   -Iutils \
# 		   -Iworld
CXXFLAGS = -g -std=c++17 -Wall -MMD -march=native -mtune=native -O2 -DNDEBUG -ffast-math \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
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

all: main viewer measure_decision test_sequences test_explore

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

measure_decision: ${OBJECTS} measure_decision.o
	${CXX} ${CXXFLAGS} ${OBJECTS} measure_decision.o -o measure_decision ${LIBS}

test_sequences: ${OBJECTS} test_sequences.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_sequences.o -o test_sequences ${LIBS}

test_explore: ${OBJECTS} test_explore.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_explore.o -o test_explore ${LIBS}

-include ${DEPENDS} main.d viewer.d measure_decision.d test_sequences.d test_explore.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	viewer.o viewer.d viewer \
	measure_decision.o measure_decision.d measure_decision \
	test_sequences.o test_sequences.d test_sequences \
	test_explore.o test_explore.d test_explore
