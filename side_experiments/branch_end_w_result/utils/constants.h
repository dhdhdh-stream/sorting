#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const int NEW_SCOPE_MIN_NODES = 20;

const double EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN = 0.2;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
const int NEW_SCOPE_IMPROVEMENTS_PER_ITER = 2;
#else
/**
 * - large number to select good paths to prevent noise
 */
const int IMPROVEMENTS_PER_ITER = 6;
const int NEW_SCOPE_IMPROVEMENTS_PER_ITER = 40;
#endif /* MDEBUG */

const int RUN_TIMESTEPS = 24;

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */