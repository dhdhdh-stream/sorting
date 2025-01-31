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

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const double ACTIONS_PER_EXPERIMENT = 400.0;

#endif /* CONSTANTS_H */