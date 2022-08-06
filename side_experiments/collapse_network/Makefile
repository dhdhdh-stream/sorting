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

all: main f2

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

f2: ${OBJECTS} f2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} f2.o -o f2 ${LIBS}

-include ${DEPENDS} main.d f2.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	f2.o f2.d f2
