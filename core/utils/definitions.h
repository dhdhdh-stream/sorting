#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

#include "solution.h"

extern std::default_random_engine generator;

extern bool global_debug_flag;
extern double global_sum_error;

const int STEP_TYPE_STEP = 0;
const int STEP_TYPE_BRANCH = 1;
const int STEP_TYPE_FOLD = 2;

const int EXPLORE_TYPE_NONE = -1;
const int EXPLORE_TYPE_INNER_SCOPE = 0;
const int EXPLORE_TYPE_INNER_BRANCH = 1;
const int EXPLORE_TYPE_NEW = 2;
// can't explore within a fold (that hasn't been fully processed)

const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_EXPLORE = 1;
// on explore, clear state but then let it be updated and decisions made as previous (there's no right approach anyways)
const int EXPLORE_PHASE_FLAT = 2;

const int EXPLORE_SIGNAL_NONE = 0;
const int EXPLORE_SIGNAL_REPLACE = 1;
const int EXPLORE_SIGNAL_BRANCH = 2;
const int EXPLORE_SIGNAL_CLEAN = 3;

extern Solution* solution;

#endif /* DEFINITIONS_H */