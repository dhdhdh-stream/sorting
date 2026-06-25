CXX = g++
# CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
# 		   `pkg-config --cflags eigen3` \
# 		   -DMDEBUG=0 \
# 		   -Inn \
# 		   -Iutils
# CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD -march=native -O2 -DNDEBUG -ffast-math \
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

all: main test_eigen

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_eigen: ${OBJECTS} test_eigen.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_eigen.o -o test_eigen ${LIBS}

-include ${DEPENDS} main.d test_eigen.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_eigen.o test_eigen.d test_eigen
