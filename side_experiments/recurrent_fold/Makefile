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

all: p2_front p2_front_viewer p2_back

p2_front: ${OBJECTS} p2_front.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2_front.o -o p2_front ${LIBS}

p2_front_viewer: ${OBJECTS} p2_front_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2_front_viewer.o -o p2_front_viewer ${LIBS}

p2_back: ${OBJECTS} p2_back.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2_back.o -o p2_back ${LIBS}

-include ${DEPENDS} p2_front.d p2_front_viewer.d p2_back.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p2_front.o p2_front.d p2_front \
	p2_front_viewer.o p2_front_viewer.d p2_front_viewer \
	p2_back.o p2_back.d p2_back
