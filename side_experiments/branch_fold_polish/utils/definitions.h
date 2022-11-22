#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

extern std::default_random_engine generator;

extern double global_sum_error;

const int EXPLORE_TYPE_NONE = -1;
const int EXPLORE_TYPE_INNER_SCOPE = 0;
const int EXPLORE_TYPE_INNER_BRANCH = 1;
const int EXPLORE_TYPE_LOCAL = 2;

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_FLAT = 1;
const int EXPLORE_PHASE_FOLD = 2;	// while folding back, also adjust front
const int EXPLORE_PHASE_ORIGINAL = 3;
const int EXPLORE_PHASE_BACKPROP_FRONT = 4;

#endif /* DEFINITIONS_H */