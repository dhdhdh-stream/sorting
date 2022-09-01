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

all: main flat rnn

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

flat: ${OBJECTS} flat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} flat.o -o flat ${LIBS}

rnn: ${OBJECTS} rnn.o
	${CXX} ${CXXFLAGS} ${OBJECTS} rnn.o -o rnn ${LIBS}

-include ${DEPENDS} main.d flat.d rnn.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	flat.o flat.d flat \
	rnn.o rnn.d rnn
