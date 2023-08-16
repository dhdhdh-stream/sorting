CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Isolution \
		   -Isolution/branch_experiment_steps \
		   -Isolution/loop_experiment_steps \
		   -Isolution/sequence_helpers \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
		  $(wildcard solution/branch_experiment_steps/*.cpp) \
		  $(wildcard solution/loop_experiment_steps/*.cpp) \
		  $(wildcard solution/sequence_helpers/*.cpp) \
		  $(wildcard utils/*.cpp) \
		  $(wildcard world/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: loop_p1 loop_p1_p2 loop_p2

loop_p1: ${OBJECTS} loop_p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} loop_p1.o -o loop_p1 ${LIBS}

loop_p1_p2: ${OBJECTS} loop_p1_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} loop_p1_p2.o -o loop_p1_p2 ${LIBS}

loop_p2: ${OBJECTS} loop_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} loop_p2.o -o loop_p2 ${LIBS}

-include ${DEPENDS} loop_p1.d loop_p1_p2.d loop_p2.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	loop_p1.o loop_p1.d loop_p1 \
	loop_p1_p2.o loop_p1_p2.d loop_p1_p2 \
	loop_p2.o loop_p2.d loop_p2
