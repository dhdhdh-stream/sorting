CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   -Isolution \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard solution/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: single

single: ${OBJECTS} single.o
	${CXX} ${CXXFLAGS} ${OBJECTS} single.o -o single ${LIBS}

-include ${DEPENDS} single.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	single.o single.d single
