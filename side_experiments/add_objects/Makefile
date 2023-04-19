CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Isolution \
		   -Isolution/fold_steps \
		   -Isolution/loop_fold_steps \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
		  $(wildcard solution/fold_steps/*.cpp) \
		  $(wildcard solution/loop_fold_steps/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: p1_front

p1_front: ${OBJECTS} p1_front.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p1_front.o -o p1_front ${LIBS}

-include ${DEPENDS} p1_front.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p1_front.o p1_front.d p1_front
