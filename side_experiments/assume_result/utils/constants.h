#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;
const int STEP_TYPE_RETURN = 2;
const int STEP_TYPE_ABSOLUTE_RETURN = 3;

/**
 * - select first that is significant improvement
 *   - don't select "best" as might not have been learned for actual best
 *     - so may select lottery instead of actual best
 */
const int EXPLORE_TYPE_GOOD = 0;
const int EXPLORE_TYPE_BEST = 1;

#if defined(MDEBUG) && MDEBUG
const int NUM_DATAPOINTS = 10;
const int MIN_NUM_TRUTH_DATAPOINTS = 10;
#else
// const int NUM_DATAPOINTS = 1000;
// const int MIN_NUM_TRUTH_DATAPOINTS = 400;

const int NUM_DATAPOINTS = 200;
const int MIN_NUM_TRUTH_DATAPOINTS = 50;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
// const int MEASURE_ITERS = 4000;

const int MEASURE_ITERS = 400;
#endif /* MDEBUG */

const int NUM_VERIFY_SAMPLES = 10;

#endif /* CONSTANTS_H */