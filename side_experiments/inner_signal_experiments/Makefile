CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=1 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: simple contrast pre_vs_post

simple: ${OBJECTS} simple.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple.o -o simple ${LIBS}

contrast: ${OBJECTS} contrast.o
	${CXX} ${CXXFLAGS} ${OBJECTS} contrast.o -o contrast ${LIBS}

pre_vs_post: ${OBJECTS} pre_vs_post.o
	${CXX} ${CXXFLAGS} ${OBJECTS} pre_vs_post.o -o pre_vs_post ${LIBS}

-include ${DEPENDS} simple.d contrast.d pre_vs_post.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	simple.o simple.d simple \
	contrast.o contrast.d contrast \
	pre_vs_post.o pre_vs_post.d pre_vs_post
