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

all: p4 p4_test_result convert_to_scope

p4: ${OBJECTS} p4.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4.o -o p4 ${LIBS}

p4_test_result: ${OBJECTS} p4_test_result.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4_test_result.o -o p4_test_result ${LIBS}

convert_to_scope: ${OBJECTS} convert_to_scope.o
	${CXX} ${CXXFLAGS} ${OBJECTS} convert_to_scope.o -o convert_to_scope ${LIBS}

-include ${DEPENDS} p4.d p4_test_result.d convert_to_scope.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p4.o p4.d p4 \
	p4_test_result.o p4_test_result.d p4_test_result \
	convert_to_scope.o convert_to_scope.d convert_to_scope
