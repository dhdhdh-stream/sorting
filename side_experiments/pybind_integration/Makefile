CXX = g++
PYBIND_INCLUDE := $(shell python3 -m pybind11 --includes)
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -shared -fPIC ${PYBIND_INCLUDE} \
		   `pkg-config --cflags eigen3` \
		   -Inn \
		   -Isolution \
		   -Iutils
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find solution -name '*.cpp') \
		   $(shell find utils -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

PYTHON_EXTENSION := $(shell python3-config --extension-suffix)

all: wrapper

wrapper: ${OBJECTS} wrapper.o
	${CXX} ${CXXFLAGS} ${OBJECTS} wrapper.o -o wrapper${PYTHON_EXTENSION} ${LIBS}

-include ${DEPENDS} wrapper.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	wrapper.o wrapper.d wrapper${PYTHON_EXTENSION}
