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
const int UPDATE_ITERS = 10;
#else
const int TRAIN_ITERS = 300000;
const int UPDATE_ITERS = 100000;
#endif /* MDEBUG */

/**
 * - simply give raw actions a fixed weight
 *   - cannot track success/count if continuous
 *   - raw actions can also drive innovation anyways
 */
const int RAW_ACTION_WEIGHT = 8;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const int NEW_SCOPE_MIN_NODES = 10;

const double TEST_SAMPLES_PERCENTAGE = 0.2;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_LAST_GLOBAL_TRACK = 4;
const int MIN_NUM_LAST_GLOBAL_TRACK = 2;
const double LAST_GLOBAL_BETTER_THAN_RATIO = 0.5;
const int NUM_LAST_LOCAL_TRACK = 4;
const int MIN_NUM_LAST_LOCAL_TRACK = 2;
const double LAST_LOCAL_BETTER_THAN_RATIO = 0.5;
const int NUM_LAST_CLEAN_TRACK = 4;
const int MIN_NUM_LAST_CLEAN_TRACK = 2;
const double LAST_CLEAN_BETTER_THAN_RATIO = 0.5;
#else
const int NUM_LAST_GLOBAL_TRACK = 10;
const int MIN_NUM_LAST_GLOBAL_TRACK = 2;
const double LAST_GLOBAL_BETTER_THAN_RATIO = 0.7;
const int NUM_LAST_LOCAL_TRACK = 10;
const int MIN_NUM_LAST_LOCAL_TRACK = 2;
const double LAST_LOCAL_BETTER_THAN_RATIO = 0.7;
const int NUM_LAST_CLEAN_TRACK = 10;
const int MIN_NUM_LAST_CLEAN_TRACK = 4;
const double LAST_CLEAN_BETTER_THAN_RATIO = 0.8;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_MAX_SAMPLES = 10;
#else
const int EXPLORE_MAX_SAMPLES = 10000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */