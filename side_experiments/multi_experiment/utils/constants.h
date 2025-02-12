#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const int GATHER_ITERS = 10;
const int GATHER_FACTORS_PER_ITER = 5;
/**
 * - will lead to factors with duplicate effects being created
 *   - but OK as getting limited regardless
 */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

const double TEST_SAMPLES_PERCENTAGE = 0.2;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;
const double REGRESSION_FAIL_MULTIPLIER = 1000.0;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

// const double ACTIONS_PER_EXPERIMENT = 400.0;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_PER_NEW_SCOPE = 2;
#else
const int BRANCH_PER_NEW_SCOPE = 10;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
const int IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */