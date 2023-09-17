CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   `pkg-config --cflags eigen3` \
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

all: main

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

-include ${DEPENDS} main.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main
