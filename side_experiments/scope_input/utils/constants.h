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

const double TEST_SAMPLES_PERCENTAGE = 0.2;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;
const double REGRESSION_FAIL_MULTIPLIER = 1000.0;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const int NETWORK_NUM_INPUTS = 10;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
/**
 * - need large number of samples
 *   - otherwise trapped by lottery + local maxima
 */
const int MEASURE_ITERS = 4000;
#endif /* MDEBUG */

const int EXPLORE_ITERS = 60;

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
const int IMPROVEMENTS_PER_ITER = 20;
#endif /* MDEBUG */

const int NEW_SCOPE_ITERS = 15;
const int EXPERIMENT_ALL_ITERS = 10;

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */