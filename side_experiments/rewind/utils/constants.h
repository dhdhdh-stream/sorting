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

#if defined(MDEBUG) && MDEBUG
const int MEASURE_STEP_NUM_ITERS = 10;
#else
const int MEASURE_STEP_NUM_ITERS = 100;
#endif /* MDEBUG */

const int BRANCH_RATIO_CHECK_ITER = 1000;
const double BRANCH_MIN_RATIO = 0.03;

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
const int NUM_LAST_TRACK = 40;
const int MIN_NUM_LAST_TRACK = 5;
const double LAST_BETTER_THAN_RATIO = 0.8;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */