#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - lean towards being more selective
 */
const double MATCH_WEIGHT = 0.5;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const double EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN = 0.2;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
/**
 * - need large number of samples
 *   - otherwise trapped by lottery + local maxima
 */
const int MEASURE_ITERS = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int IMPROVEMENTS_PER_ITER = 2;
#else
/**
 * - large number to try to mitigate bad variance
 * 
 * - most important is to prevent backsliding
 *   - there are safe, powerful ways to break local optima
 *   - but if can backslide, then likely will backslide to nothing
 */
const int IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_TARGET_NUM_SAMPLES = 10;
#else
const int EXPLORE_TARGET_NUM_SAMPLES = 4000;
#endif /* MDEBUG */

const int MAX_NUM_SOLUTIONS = 40;

#if defined(MDEBUG) && MDEBUG
const int NUM_VERIFY_SAMPLES = 10;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */