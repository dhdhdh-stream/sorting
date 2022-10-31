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

all: p7_branch_combine

p7_branch_combine: ${OBJECTS} p7_branch_combine.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p7_branch_combine.o -o p7_branch_combine ${LIBS}

-include ${DEPENDS} p7_branch_combine.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p7_branch_combine.o p7_branch_combine.d p7_branch_combine
