#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

extern std::default_random_engine generator;

extern double global_sum_error;

const int EXPLORE_TYPE_NONE = -1;
const int EXPLORE_TYPE_INNER = 0;
const int EXPLORE_TYPE_LOCAL = 1;

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_LEARN = 1;
const int EXPLORE_PHASE_MEASURE = 2;
const int EXPLORE_PHASE_FRONT = 3;

#endif /* DEFINITIONS_H */