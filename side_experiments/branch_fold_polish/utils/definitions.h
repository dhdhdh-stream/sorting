#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

extern std::default_random_engine generator;

extern double global_sum_error;

const int EXPLORE_TYPE_NEW = 0;
const int EXPLORE_TYPE_SCOPE = 1;
const int EXPLORE_TYPE_BRANCH = 2;

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_LEARN = 1;
const int EXPLORE_PHASE_MEASURE = 2;

#endif /* DEFINITIONS_H */