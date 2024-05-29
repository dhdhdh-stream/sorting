CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main no_cluster cluster

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

no_cluster: ${OBJECTS} no_cluster.o
	${CXX} ${CXXFLAGS} ${OBJECTS} no_cluster.o -o no_cluster ${LIBS}

cluster: ${OBJECTS} cluster.o
	${CXX} ${CXXFLAGS} ${OBJECTS} cluster.o -o cluster ${LIBS}

-include ${DEPENDS} main.d no_cluster.d cluster.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	no_cluster.o no_cluster.d no_cluster \
	cluster.o cluster.d cluster
