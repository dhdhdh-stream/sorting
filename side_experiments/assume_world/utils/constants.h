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
const int EXPLORE_TYPE_NEUTRAL = 1;
const int EXPLORE_TYPE_BEST = 2;

const int MAX_EXPLORE_TRIES = 4;

const double TEST_SAMPLES_PERCENTAGE = 0.2;

#if defined(MDEBUG) && MDEBUG
const int NUM_DATAPOINTS = 10;
const int MIN_NUM_TRUTH_DATAPOINTS = 10;
const int VERIFY_NUM_DATAPOINTS = 10;
#else
const int NUM_DATAPOINTS = 4000;
const int MIN_NUM_TRUTH_DATAPOINTS = 200;
const int VERIFY_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 4000;
#endif /* MDEBUG */

const int NUM_VERIFY_SAMPLES = 10;

const int MIN_ITERS_BEFORE_NEXT_NEW_SCOPE = 6;

#endif /* CONSTANTS_H */