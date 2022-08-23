CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD -lboost_system \
		   -Iexplore \
		   -Inn \
		   -Isolution \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES = $(wildcard explore/*.cpp) \
		  $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
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
