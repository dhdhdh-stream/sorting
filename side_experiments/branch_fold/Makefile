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

all: p5 p5_test_result

p5: ${OBJECTS} p5.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p5.o -o p5 ${LIBS}

p5_test_result: ${OBJECTS} p5_test_result.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p5_test_result.o -o p5_test_result ${LIBS}

-include ${DEPENDS} p5.d p5_test_result.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p5.o p5.d p5 \
	p5_test_result.o p5_test_result.d p5_test_result
