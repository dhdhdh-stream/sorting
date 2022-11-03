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

all: p8_compound_action

p8_compound_action: ${OBJECTS} p8_compound_action.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p8_compound_action.o -o p8_compound_action ${LIBS}

-include ${DEPENDS} p8_compound_action.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p8_compound_action.o p8_compound_action.d p8_compound_action
