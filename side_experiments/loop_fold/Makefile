CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD -lboost_system \
		   -Inn \
		   -Isolution \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: p3

p3: ${OBJECTS} p3.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3.o -o p3 ${LIBS}

-include ${DEPENDS} p3.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p3.o p3.d p3
