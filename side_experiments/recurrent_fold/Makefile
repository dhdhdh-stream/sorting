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

all: p3_front

p3_front: ${OBJECTS} p3_front.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_front.o -o p3_front ${LIBS}

-include ${DEPENDS} p3_front.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p3_front.o p3_front.d p3_front
