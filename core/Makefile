CXX = g++
CXXFLAGS = -g -std=c++14 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Isolver \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solver/*.cpp) \
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
	rm -f ${DEPENDS} ${DEPENDS:.d=.o} \
	main.o main
