CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils \
		   -Iworld \
		   -Iworld_model
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp') \
		   $(shell find world_model -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main big_flat

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

big_flat: ${OBJECTS} big_flat.o
	${CXX} ${CXXFLAGS} ${OBJECTS} big_flat.o -o big_flat ${LIBS}

-include ${DEPENDS} main.d big_flat.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	big_flat.o big_flat.d big_flat
