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
const int EXPLORE_PHASE_MEASURE = 2;
// if EXPLORE_PHASE_NONE till end, then potentially multiple folds, so update score networks along path till start

// Note: safe to update scores/branch mods even with flats in progress, as the flats will be scaled by the mods as well

#endif /* DEFINITIONS_H */