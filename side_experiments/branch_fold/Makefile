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

all: p7

p7: ${OBJECTS} p7.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p7.o -o p7 ${LIBS}

# p7_measure_weight: ${OBJECTS} p7_measure_weight.o
# 	${CXX} ${CXXFLAGS} ${OBJECTS} p7_measure_weight.o -o p7_measure_weight ${LIBS}

# p7_test_result: ${OBJECTS} p7_test_result.o
# 	${CXX} ${CXXFLAGS} ${OBJECTS} p7_test_result.o -o p7_test_result ${LIBS}

-include ${DEPENDS} p7.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p7.o p7.d p7
