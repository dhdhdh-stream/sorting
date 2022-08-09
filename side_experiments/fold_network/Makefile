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

all: main eval_sort learn_sort main_compare eval_sort_compare

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

eval_sort: ${OBJECTS} eval_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} eval_sort.o -o eval_sort ${LIBS}

learn_sort: ${OBJECTS} learn_sort.o
	${CXX} ${CXXFLAGS} ${OBJECTS} learn_sort.o -o learn_sort ${LIBS}

main_compare: ${OBJECTS} main_compare.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main_compare.o -o main_compare ${LIBS}

eval_sort_compare: ${OBJECTS} eval_sort_compare.o
	${CXX} ${CXXFLAGS} ${OBJECTS} eval_sort_compare.o -o eval_sort_compare ${LIBS}

-include ${DEPENDS} main.d eval_sort.d learn_sort.d main_compare.d eval_sort_compare.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	eval_sort.o eval_sort.d eval_sort \
	learn_sort.o learn_sort.d learn_sort \
	main_compare.o main_compare.d main_compare \
	eval_sort_compare.o eval_sort_compare.d eval_sort_compare
