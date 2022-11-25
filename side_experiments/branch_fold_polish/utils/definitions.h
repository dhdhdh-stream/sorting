#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

extern std::default_random_engine generator;

extern double global_sum_error;

const int EXPLORE_TYPE_NONE = -1;
const int EXPLORE_TYPE_INNER_SCOPE = 0;
const int EXPLORE_TYPE_INNER_BRANCH = 1;
const int EXPLORE_TYPE_NEW = 2;	// after flat, becomes EXPLORE_TYPE_INNER_BRANCH

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_FLAT = 1;
const int EXPLORE_PHASE_MEASURE = 2;	// if measure, don't need to save anything, but also not bothering to special case for now

// Note: update scores even with flats in progress (hopefully, effect negligible, or flat can adjust)

#endif /* DEFINITIONS_H */