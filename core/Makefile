CXX = g++
CXXFLAGS = -g -std=c++17 -Wall -Werror -MMD \
		   -DMDEBUG=0 \
		   `pkg-config --cflags eigen3` \
		   -Inn \
		   -Isolution \
		   -Iutils \
		   -Iworld
LIBS = -pthread
SOURCES := $(shell find nn -name '*.cpp') \
		   $(shell find solution -name '*.cpp') \
		   $(shell find utils -name '*.cpp') \
		   $(shell find world -name '*.cpp')
HEADERS = ${SOURCES:.cpp=.h}
OBJECTS = ${HEADERS:.h=.o}
DEPENDS = ${OBJECTS:.o=.d}

all: main viewer worker simple_init measure step_through suggestion train_initial_action train_initial_action_worker setup_initial_action prepare_second_action train_second_action train_second_action_worker

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

viewer: ${OBJECTS} viewer.o
	${CXX} ${CXXFLAGS} ${OBJECTS} viewer.o -o viewer ${LIBS}

worker: ${OBJECTS} worker.o
	${CXX} ${CXXFLAGS} ${OBJECTS} worker.o -o worker ${LIBS}

simple_init: ${OBJECTS} simple_init.o
	${CXX} ${CXXFLAGS} ${OBJECTS} simple_init.o -o simple_init ${LIBS}

measure: ${OBJECTS} measure.o
	${CXX} ${CXXFLAGS} ${OBJECTS} measure.o -o measure ${LIBS}

step_through: ${OBJECTS} step_through.o
	${CXX} ${CXXFLAGS} ${OBJECTS} step_through.o -o step_through ${LIBS}

suggestion: ${OBJECTS} suggestion.o
	${CXX} ${CXXFLAGS} ${OBJECTS} suggestion.o -o suggestion ${LIBS}

train_initial_action: ${OBJECTS} train_initial_action.o
	${CXX} ${CXXFLAGS} ${OBJECTS} train_initial_action.o -o train_initial_action ${LIBS}

train_initial_action_worker: ${OBJECTS} train_initial_action_worker.o
	${CXX} ${CXXFLAGS} ${OBJECTS} train_initial_action_worker.o -o train_initial_action_worker ${LIBS}

setup_initial_action: ${OBJECTS} setup_initial_action.o
	${CXX} ${CXXFLAGS} ${OBJECTS} setup_initial_action.o -o setup_initial_action ${LIBS}

prepare_second_action: ${OBJECTS} prepare_second_action.o
	${CXX} ${CXXFLAGS} ${OBJECTS} prepare_second_action.o -o prepare_second_action ${LIBS}

train_second_action: ${OBJECTS} train_second_action.o
	${CXX} ${CXXFLAGS} ${OBJECTS} train_second_action.o -o train_second_action ${LIBS}

train_second_action_worker: ${OBJECTS} train_second_action_worker.o
	${CXX} ${CXXFLAGS} ${OBJECTS} train_second_action_worker.o -o train_second_action_worker ${LIBS}

-include ${DEPENDS} main.d viewer.d worker.d simple_init.d measure.d step_through.d suggestion.d train_initial_action.d train_initial_action_worker.d setup_initial_action.d prepare_second_action.d train_second_action.d train_second_action_worker.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	viewer.o viewer.d viewer \
	worker.o worker.d worker \
	simple_init.o simple_init.d simple_init \
	measure.o measure.d measure \
	step_through.o step_through.d step_through \
	suggestion.o suggestion.d suggestion \
	train_initial_action.o train_initial_action.d train_initial_action \
	train_initial_action_worker.o train_initial_action_worker.d train_initial_action_worker \
	setup_initial_action.o setup_initial_action.d setup_initial_action \
	prepare_second_action.o prepare_second_action.d prepare_second_action \
	train_second_action.o train_second_action.d train_second_action \
	train_second_action_worker.o train_second_action_worker.d train_second_action_worker
