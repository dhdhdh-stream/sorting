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

const int LINEAR_NUM_OBS = 50;
const int NETWORK_INCREMENT_NUM_NEW = 10;

/**
 * - Eigen SVD can assign meaningless large weights?
 *   - can cause rounding errors after saving/loading
 *     - will try both truncating and catching?
 */
const double LINEAR_MAX_WEIGHT = 100.0;

const double TEST_SAMPLES_PERCENTAGE = 0.3;

const double WEIGHT_MIN_SCORE_IMPACT = 0.1;

const double EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT = 0.05;
const double PASS_THROUGH_BRANCH_WEIGHT = 0.9;

/**
 * - if scope activated more than MAX_INSTANCES_PER_RUN, restrict, and assume multiple explore/new doesn't harm eval accuracy too badly
 *   - restrict to increase speed
 */
const int MAX_INSTANCES_PER_RUN = 10;

const int MAX_NUM_ACTIONS_LIMIT_MULTIPLIER = 5;

/**
 * TODO: can potentially reduce NUM_DATAPOINTS with eval?
 */
#if defined(MDEBUG) && MDEBUG
const int NUM_DATAPOINTS = 10;
const int VERIFY_1ST_NUM_DATAPOINTS = 10;
const int VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NUM_DATAPOINTS = 2000;
const int VERIFY_1ST_NUM_DATAPOINTS = 4000;
const int VERIFY_2ND_NUM_DATAPOINTS = 8000;
#endif /* MDEBUG */

const int NUM_VERIFY_SAMPLES = 10;

#endif /* CONSTANTS_H */