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

all: main eval_sort learn_sort

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

eval_sort: ${OBJECTS} eval_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} eval_sort.o -o eval_sort ${LIBS}

learn_sort: ${OBJECTS} learn_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} learn_sort.o -o learn_sort ${LIBS}

-include ${DEPENDS} main.d eval_sort.d learn_sort.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	eval_sort.o eval_sort.d eval_sort \
	learn_sort.o learn_sort.d learn_sort
