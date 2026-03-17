#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

/**
 * - simply give raw actions a fixed weight
 *   - cannot track success/count if continuous
 *   - raw actions can also drive innovation anyways
 */
const int RAW_ACTION_WEIGHT = 8;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const int NEW_SCOPE_MIN_NODES = 10;

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

const double BRANCH_MIN_RATIO = 0.05;

const double FAIL_T_SCORE = 0.674;
const double SUCCESS_T_SCORE = 2.326;

#if defined(MDEBUG) && MDEBUG
const int NUM_LAST_TRACK = 4;
const int MIN_NUM_LAST_TRACK = 2;
const double LAST_BETTER_THAN_RATIO = 0.5;
#else
/**
 * - track relatively large number
 *   - to help prevent string of bad luck from adding bad changes
 */
const int NUM_LAST_TRACK = 10;
const int MIN_NUM_LAST_TRACK = 5;
const double LAST_BETTER_THAN_RATIO = 0.8;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int HISTORIES_NUM_SAVE = 100;
#else
const int HISTORIES_NUM_SAVE = 4000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */