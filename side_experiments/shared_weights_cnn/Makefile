CXX = g++
CXXFLAGS = -g -std=c++14 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Iutils \
		   -Iworld
LIBS = -pthread -lboost_system
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard utils/*.cpp) \
		  $(wildcard world/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: learn_sort learn_to_halt test_limits

learn_sort: ${OBJECTS} learn_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} learn_sort.o -o learn_sort ${LIBS}

learn_to_halt: ${OBJECTS} learn_to_halt.o
	${CXX} ${CXXFLAGS} ${OBJECTS} learn_to_halt.o -o learn_to_halt ${LIBS}

test_limits: ${OBJECTS} test_limits.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_limits.o -o test_limits ${LIBS}

-include ${DEPENDS} learn_sort.d learn_to_halt.d test_limits.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	learn_sort.o learn_sort.d learn_sort \
	learn_to_halt.o learn_to_halt.d learn_to_halt \
	test_limits.o test_limits.d test_limits
