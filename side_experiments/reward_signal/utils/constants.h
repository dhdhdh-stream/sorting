#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const double EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN = 0.2;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_S1_ITERS = 1;
const int MEASURE_S2_ITERS = 2;
const int MEASURE_S3_ITERS = 5;
const int MEASURE_S4_ITERS = 10;
#else
const int MEASURE_S1_ITERS = 1;
const int MEASURE_S2_ITERS = 10;
const int MEASURE_S3_ITERS = 100;
const int MEASURE_S4_ITERS = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int EARLY_SUCCESS_S1_ITERS = 2;
const int EARLY_SUCCESS_S2_ITERS = 5;
#else
const int EARLY_SUCCESS_S1_ITERS = 100;
const int EARLY_SUCCESS_S2_ITERS = 400;
#endif /* MDEBUG */
const double EARLY_SUCCESS_MIN_T_SCORE = 3.0;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
const int NUM_EXPLORE_SAVE = 5;
#else
/**
 * - need large number of samples
 *   - otherwise trapped by lottery + local maxima
 */
const int MEASURE_ITERS = 1000;
const int NUM_EXPLORE_SAVE = 500;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
/**
 * - large number to select good paths to prevent noise
 */
const int IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */