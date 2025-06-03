#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

/**
 * - select first that is significant improvement
 *   - don't select "best" as might not have been learned for actual best
 *     - so may select lottery instead of actual best
 */
const int EXPLORE_TYPE_GOOD = 0;
const int EXPLORE_TYPE_BEST = 1;

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

const int CHECK_EXPERIMENT_ITER = 100;
// const int ACTIONS_PER_EXPERIMENT = 20;
const int ACTIONS_PER_EXPERIMENT = 10;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
/**
 * - need large number of samples
 *   - otherwise trapped by lottery + local maxima
 */
const int MEASURE_ITERS = 4000;
#endif /* MDEBUG */

const int SCOPE_EXCEEDED_NUM_NODES = 80;
const int SCOPE_RESUME_NUM_NODES = 40;

const int EXPLORE_ITERS = 60;

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
const int IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */