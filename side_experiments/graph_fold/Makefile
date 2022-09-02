CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD -lboost_system \
		   -Igraph \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard graph/*.cpp) \
		  $(wildcard nn/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main flat rnn new_path_test mini_fold

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

flat: ${OBJECTS} flat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} flat.o -o flat ${LIBS}

rnn: ${OBJECTS} rnn.o
	${CXX} ${CXXFLAGS} ${OBJECTS} rnn.o -o rnn ${LIBS}

new_path_test: ${OBJECTS} new_path_test.o
	${CXX} ${CXXFLAGS} ${OBJECTS} new_path_test.o -o new_path_test ${LIBS}

mini_fold: ${OBJECTS} mini_fold.o
	${CXX} ${CXXFLAGS} ${OBJECTS} mini_fold.o -o mini_fold ${LIBS}

-include ${DEPENDS} main.d flat.d rnn.d new_path_test.d mini_fold.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	flat.o flat.d flat \
	rnn.o rnn.d rnn \
	new_path_test.o new_path_test.d new_path_test \
	mini_fold.o mini_fold.d mini_fold
