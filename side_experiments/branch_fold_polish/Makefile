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

all: p2 p2_viewer

p2: ${OBJECTS} p2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2.o -o p2 ${LIBS}

p2_viewer: ${OBJECTS} p2_viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} p2_viewer.o -o p2_viewer ${LIBS}

-include ${DEPENDS} p2.d p2_viewer.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	p2.o p2.d p2 \
	p2_viewer.o p2_viewer.d p2_viewer \
