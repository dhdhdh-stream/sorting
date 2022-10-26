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

all: p6 p6_test_result p6_branch_front p6_through_back

p6: ${OBJECTS} p6.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p6.o -o p6 ${LIBS}

p6_test_result: ${OBJECTS} p6_test_result.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p6_test_result.o -o p6_test_result ${LIBS}

p6_branch_front: ${OBJECTS} p6_branch_front.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p6_branch_front.o -o p6_branch_front ${LIBS}

p6_through_back: ${OBJECTS} p6_through_back.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p6_through_back.o -o p6_through_back ${LIBS}

-include ${DEPENDS} p6.d p6_test_result.d p6_branch_front.d p6_through_back.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p6.o p6.d p6 \
	p6_test_result.o p6_test_result.d p6_test_result \
	p6_branch_front.o p6_branch_front.d p6_branch_front \
	p6_through_back.o p6_through_back.d p6_through_back
