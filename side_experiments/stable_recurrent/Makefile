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

all: main test_y_negate test_n_negate shift

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_y_negate: ${OBJECTS} test_y_negate.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_y_negate.o -o test_y_negate ${LIBS}

test_n_negate: ${OBJECTS} test_n_negate.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_n_negate.o -o test_n_negate ${LIBS}

shift: ${OBJECTS} shift.o
	${CXX} ${CXXFLAGS} ${OBJECTS} shift.o -o shift ${LIBS}

-include ${DEPENDS} main.d test_y_negate.d test_n_negate.d shift.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_y_negate.o test_y_negate.d test_y_negate \
	test_n_negate.o test_n_negate.d test_n_negate \
	shift.o shift.d shift
