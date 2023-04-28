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

all: p1_front p1_front_viewer p1_back

p1_front: ${OBJECTS} p1_front.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p1_front.o -o p1_front ${LIBS}

p1_front_viewer: ${OBJECTS} p1_front_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p1_front_viewer.o -o p1_front_viewer ${LIBS}

p1_back: ${OBJECTS} p1_back.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p1_back.o -o p1_back ${LIBS}

-include ${DEPENDS} p1_front.d p1_front_viewer.d p1_back.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p1_front.o p1_front.d p1_front \
	p1_front_viewer.o p1_front_viewer.d p1_front_viewer \
	p1_back.o p1_back.d p1_back
