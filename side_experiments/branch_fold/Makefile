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

all: main test_result

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_result: ${OBJECTS} test_result.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_result.o -o test_result ${LIBS}

-include ${DEPENDS} main.d test_result.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_result.o test_result.d test_result
