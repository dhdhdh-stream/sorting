#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
const int UPDATE_ITERS = 10;
#else
const int TRAIN_ITERS = 300000;
const int UPDATE_ITERS = 200000;
#endif /* MDEBUG */

/**
 * - simply give raw actions a fixed weight
 *   - cannot track success/count if continuous
 *   - raw actions can also drive innovation anyways
 */
const int RAW_ACTION_WEIGHT = 8;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const double CONSISTENCY_MIN_WEIGHT = 0.5;

const int NEW_SCOPE_MIN_NODES = 10;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 1000;
#endif /* MDEBUG */

/**
 * - large number to select good paths to prevent noise
 */
#if defined(MDEBUG) && MDEBUG
const int BRANCH_IMPROVEMENTS_PER_ITER = 2;
#else
const int BRANCH_IMPROVEMENTS_PER_ITER = 6;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */