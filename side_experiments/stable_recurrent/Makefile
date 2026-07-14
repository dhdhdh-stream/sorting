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

all: main test_y_negate test_n_negate shift just_in_time just_in_time_t2 w_o_drop

main: ${OBJECTS} main.o
	${CXX} ${CXXFLAGS} ${OBJECTS} main.o -o main ${LIBS}

test_y_negate: ${OBJECTS} test_y_negate.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_y_negate.o -o test_y_negate ${LIBS}

test_n_negate: ${OBJECTS} test_n_negate.o
	${CXX} ${CXXFLAGS} ${OBJECTS} test_n_negate.o -o test_n_negate ${LIBS}

shift: ${OBJECTS} shift.o
	${CXX} ${CXXFLAGS} ${OBJECTS} shift.o -o shift ${LIBS}

just_in_time: ${OBJECTS} just_in_time.o
	${CXX} ${CXXFLAGS} ${OBJECTS} just_in_time.o -o just_in_time ${LIBS}

just_in_time_t2: ${OBJECTS} just_in_time_t2.o
	${CXX} ${CXXFLAGS} ${OBJECTS} just_in_time_t2.o -o just_in_time_t2 ${LIBS}

w_o_drop: ${OBJECTS} w_o_drop.o
	${CXX} ${CXXFLAGS} ${OBJECTS} w_o_drop.o -o w_o_drop ${LIBS}

-include ${DEPENDS} main.d test_y_negate.d test_n_negate.d shift.d just_in_time.d just_in_time_t2.d w_o_drop.d

clean:
	rm -f ${OBJECTS} ${DEPENDS} \
	main.o main.d main \
	test_y_negate.o test_y_negate.d test_y_negate \
	test_n_negate.o test_n_negate.d test_n_negate \
	shift.o shift.d shift \
	just_in_time.o just_in_time.d just_in_time \
	just_in_time_t2.o just_in_time_t2.d just_in_time_t2 \
	w_o_drop.o w_o_drop.d w_o_drop
