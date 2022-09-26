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

all: main test_result p2 test_result_p2 relearn_end calc_explore_weights zero_train test_zero_train

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_result: ${OBJECTS} test_result.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_result.o -o test_result ${LIBS}

p2: ${OBJECTS} p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2.o -o p2 ${LIBS}

test_result_p2: ${OBJECTS} test_result_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_result_p2.o -o test_result_p2 ${LIBS}

relearn_end: ${OBJECTS} relearn_end.o
	${CXX} ${CXXFLAGS} ${OBJECTS} relearn_end.o -o relearn_end ${LIBS}

calc_explore_weights: ${OBJECTS} calc_explore_weights.o
	${CXX} ${CXXFLAGS} ${OBJECTS} calc_explore_weights.o -o calc_explore_weights ${LIBS}

zero_train: ${OBJECTS} zero_train.o
	${CXX} ${CXXFLAGS} ${OBJECTS} zero_train.o -o zero_train ${LIBS}

test_zero_train: ${OBJECTS} test_zero_train.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_zero_train.o -o test_zero_train ${LIBS}

-include ${DEPENDS} p2.d test_result_p2.d relearn_end.d calc_explore_weights.d zero_train.d test_zero_train.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_result.o test_result.d test_result \
	p2.o p2.d p2 \
	test_result_p2.o test_result_p2.d test_result_p2 \
	relearn_end.o relearn_end.d relearn_end \
	calc_explore_weights.o calc_explore_weights.d calc_explore_weights \
	zero_train.o zero_train.d zero_train \
	test_zero_train.o test_zero_train.d test_zero_train
