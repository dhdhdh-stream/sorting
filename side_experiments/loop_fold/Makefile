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

all: p3 p4 p4_direct

p3: ${OBJECTS} p3.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3.o -o p3 ${LIBS}

p4: ${OBJECTS} p4.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4.o -o p4 ${LIBS}

p4_direct: ${OBJECTS} p4_direct.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4_direct.o -o p4_direct ${LIBS}

-include ${DEPENDS} p3.d p4.d p4_direct.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p3.o p3.d p3 \
	p4.o p4.d p4 \
	p4_direct.o p4_direct.d p4_direct
