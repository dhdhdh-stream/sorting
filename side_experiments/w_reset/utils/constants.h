#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_DATAPOINTS = 20;
#else
const int EXPERIMENT_NUM_DATAPOINTS = 5000;
#endif /* MDEBUG */
const double VERIFY_RATIO = 0.2;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

/**
 * - simply give raw actions a fixed weight
 *   - cannot track success/count if continuous
 *   - raw actions can also drive innovation anyways
 */
const int RAW_ACTION_WEIGHT = 8;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

#if defined(MDEBUG) && MDEBUG
const int NUM_LAST_TRACK = 4;
const int MIN_NUM_LAST_TRACK = 2;
const double LAST_BETTER_THAN_RATIO = 0.5;
#else
const int NUM_LAST_TRACK = 10;
const int MIN_NUM_LAST_TRACK = 5;
const double LAST_BETTER_THAN_RATIO = 0.8;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int RAMP_LOWER = -5;
const int RAMP_UPPER = 20;
const int UPDATE_NUM_ITERS = 40;
const int BATCH_SIZE = 10;
const int ITERS_PER_BATCH = 10;
#else
const int RAMP_LOWER = -5000;
const int RAMP_UPPER = 20000;
const int UPDATE_NUM_ITERS = 40000;
const int BATCH_SIZE = 1000;
const int ITERS_PER_BATCH = 10000;
#endif /* MDEBUG */

const int GENERALIZE_ITER = 3;

const int EXPERIMENT_REFRESH_NUM_ITERS = 10;

const int STUCK_NUM_ITERS = 12;

#endif /* CONSTANTS_H */