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

all: p4_p1 p4_p1_viewer

p4_p1: ${OBJECTS} p4_p1.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4_p1.o -o p4_p1 ${LIBS}

p4_p1_viewer: ${OBJECTS} p4_p1_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p4_p1_viewer.o -o p4_p1_viewer ${LIBS}

-include ${DEPENDS} p4_p1.d p4_p1_viewer.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p4_p1.o p4_p1.d p4_p1 \
	p4_p1_viewer.o p4_p1_viewer.d p4_p1_viewer
