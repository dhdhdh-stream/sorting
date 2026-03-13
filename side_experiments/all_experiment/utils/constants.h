#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

/**
 * - always give raw actions a large weight
 *   - existing scopes often learned to avoid certain patterns
 *     - which can prevent innovation
 */
const int RAW_ACTION_WEIGHT = 10;

#if defined(MDEBUG) && MDEBUG
const int HISTORIES_NUM_SAVE = 100;
#else
const int HISTORIES_NUM_SAVE = 4000;
#endif /* MDEBUG */

const int LAST_NUM_TRACK = 1000;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 5;
const int MEASURE_GEAR = 2;
const int RAMP_EPOCH_NUM_ITERS = 40;
const int MEASURE_STEP_NUM_ITERS = 40;
#else
const int EXPERIMENT_NUM_GEARS = 9;
const int MEASURE_GEAR = 4;
const int RAMP_EPOCH_NUM_ITERS = 4000;
const int MEASURE_STEP_NUM_ITERS = 4000;
#endif /* MDEBUG */

const int BRANCH_RATIO_CHECK_ITER = 200;
const double BRANCH_MIN_RATIO = 0.03;

const int NUM_LAST_TRACK = 10;
#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_LAST_TRACK = 1;
#else
const int MIN_NUM_LAST_TRACK = 5;
#endif /* MDEBUG */
const double LAST_BETTER_THAN_RATIO = 0.8;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 40;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */
const double SCOPE_EXPERIMENT_MIN_HIT = 0.3;

const int NON_OUTER_ITERS = 20;
const int OUTER_ITERS = 8;

const int NUM_ALL_ITERS = 5;
const int NUM_SCOPE_ITERS = 3;

#endif /* CONSTANTS_H */