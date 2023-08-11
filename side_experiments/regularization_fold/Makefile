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

all: main test_p1 test_p2 test_p3

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_p1: ${OBJECTS} test_p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_p1.o -o test_p1 ${LIBS}

test_p2: ${OBJECTS} test_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_p2.o -o test_p2 ${LIBS}

test_p3: ${OBJECTS} test_p3.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_p3.o -o test_p3 ${LIBS}

-include ${DEPENDS} main.d test_p1.d test_p2.d test_p3.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_p1.o test_p1.d test_p1 \
	test_p2.o test_p2.d test_p2 \
	test_p3.o test_p3.d test_p3
