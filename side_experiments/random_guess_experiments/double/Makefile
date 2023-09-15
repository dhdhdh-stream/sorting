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

all: double

double: ${OBJECTS} double.o
	${CXX} ${CXXFLAGS} ${OBJECTS} double.o -o double ${LIBS}

-include ${DEPENDS} double.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	double.o double.d double
