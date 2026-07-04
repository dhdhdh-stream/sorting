CXX = g++
# CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
# 		   `pkg-config --cflags eigen3` \
# 		   -DMDEBUG=0 \
# 		   -Inn \
# 		   -Iutils
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD -march=native -mtune=native -O2 -DNDEBUG -ffast-math \
		   `pkg-config --cflags eigen3` \
		   -DMDEBUG=0 \
		   -Inn \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main no_samples naive

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

no_samples: ${OBJECTS} no_samples.o
	${CXX} ${CXXFLAGS} ${OBJECTS} no_samples.o -o no_samples ${LIBS}

naive: ${OBJECTS} naive.o
	${CXX} ${CXXFLAGS} ${OBJECTS} naive.o -o naive ${LIBS}

-include ${DEPENDS} main.d no_samples.d naive.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	no_samples.o no_samples.d no_samples \
	naive.o naive.d naive
