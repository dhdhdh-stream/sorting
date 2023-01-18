CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -Wconversion -MMD \
		   -Inn \
		   -Isolution \
		   -Isolution/fold_steps \
		   -Iutils
LIBS = -pthread
SOURCES = $(wildcard nn/*.cpp) \
		  $(wildcard solution/*.cpp) \
		  $(wildcard solution/fold_steps/*.cpp) \
		  $(wildcard utils/*.cpp)
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: p3_p1 p3_p1_viewer p3_p2 p3_p2_viewer p3_p3

p3_p1: ${OBJECTS} p3_p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_p1.o -o p3_p1 ${LIBS}

p3_p1_viewer: ${OBJECTS} p3_p1_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_p1_viewer.o -o p3_p1_viewer ${LIBS}

p3_p2: ${OBJECTS} p3_p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_p2.o -o p3_p2 ${LIBS}

p3_p2_viewer: ${OBJECTS} p3_p2_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_p2_viewer.o -o p3_p2_viewer ${LIBS}

p3_p3: ${OBJECTS} p3_p3.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p3_p3.o -o p3_p3 ${LIBS}

-include ${DEPENDS} p3_p1.d p3_p1_viewer.d p3_p2.d p3_p2_viewer.d p3_p3.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p3_p1.o p3_p1.d p3_p1 \
	p3_p1_viewer.o p3_p1_viewer.d p3_p1_viewer \
	p3_p2.o p3_p2.d p3_p2 \
	p3_p2_viewer.o p3_p2_viewer.d p3_p2_viewer \
	p3_p3.o p3_p3.d p3_p3
