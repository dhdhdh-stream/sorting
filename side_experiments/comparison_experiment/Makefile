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

all: main sanity split_output split_state infinite_samples

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

sanity: ${OBJECTS} sanity.o
	${CXX} ${CXXFLAGS} ${OBJECTS} sanity.o -o sanity ${LIBS}

split_output: ${OBJECTS} split_output.o
	${CXX} ${CXXFLAGS} ${OBJECTS} split_output.o -o split_output ${LIBS}

split_state: ${OBJECTS} split_state.o
	${CXX} ${CXXFLAGS} ${OBJECTS} split_state.o -o split_state ${LIBS}

infinite_samples: ${OBJECTS} infinite_samples.o
	${CXX} ${CXXFLAGS} ${OBJECTS} infinite_samples.o -o infinite_samples ${LIBS}

-include ${DEPENDS} main.d sanity.d split_output.d split_state.d infinite_samples.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	sanity.o sanity.d sanity \
	split_output.o split_output.d split_output \
	split_state.o split_state.d split_state \
	infinite_samples.o infinite_samples.d infinite_samples
