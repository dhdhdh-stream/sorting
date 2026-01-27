CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Ilogic \
		   -Inn \
		   -Iproblem \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find logic -name '*.cpp') \
		   $(shell find nn -name '*.cpp') \
		   $(shell find problem -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main viewer pre_vs_post

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

pre_vs_post: ${OBJECTS} pre_vs_post.o
	${CXX} ${CXXFLAGS} ${OBJECTS} pre_vs_post.o -o pre_vs_post ${LIBS}

-include ${DEPENDS} main.d viewer.d pre_vs_post.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	viewer.o viewer.d viewer \
	pre_vs_post.o pre_vs_post.d pre_vs_post
