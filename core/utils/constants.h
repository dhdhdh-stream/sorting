#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

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

const double PASS_THROUGH_BRANCH_WEIGHT = 0.9;

#if defined(MDEBUG) && MDEBUG
const int NUM_DATAPOINTS = 10;
const int VERIFY_1ST_NUM_DATAPOINTS = 2;
const int VERIFY_2ND_NUM_DATAPOINTS = 2;
#else
const int NUM_DATAPOINTS = 2000;
const int VERIFY_1ST_NUM_DATAPOINTS = 4000;
const int VERIFY_2ND_NUM_DATAPOINTS = 8000;
#endif /* MDEBUG */

const int NUM_VERIFY_SAMPLES = 10;

#endif /* CONSTANTS_H */