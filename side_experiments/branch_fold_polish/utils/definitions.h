#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

extern std::default_random_engine generator;

extern double global_sum_error;

const int EXPLORE_TYPE_NONE = -1;
const int EXPLORE_TYPE_INNER_SCOPE = 0;
const int EXPLORE_TYPE_INNER_BRANCH = 1;
const int EXPLORE_TYPE_NEW = 2;	// after flat, becomes EXPLORE_TYPE_INNER_BRANCH
// can't explore within a fold (that hasn't been fully processed)

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_FLAT = 1;

const int EXPLORE_SIGNAL_NONE = 0;
const int EXPLORE_SIGNAL_REPLACE = 1;
const int EXPLORE_SIGNAL_BRANCH = 2;
const int EXPLORE_SIGNAL_CLEAN = 3;

#endif /* DEFINITIONS_H */