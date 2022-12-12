CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD -lboost_system \
		   -Inn \
		   -Isolution \
		   -Isolution/fold_steps \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
		  $(wildcard solution/fold_steps/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: p1

p1: ${OBJECTS} p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p1.o -o p1 ${LIBS}

-include ${DEPENDS} p1.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p1.o p1.d p1 \
