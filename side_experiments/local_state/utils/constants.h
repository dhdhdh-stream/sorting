#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const double NETWORK_INIT_MULTIPLIER = 0.01;

const int INIT_EPOCH_SIZE = 10;
const int RAMP_EPOCH_SIZE = 20;
const int UPDATE_EPOCH_SIZE = 100;

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

const int NEW_STATE_NUM_ADD = 2;

#if defined(MDEBUG) && MDEBUG
const int REUSE_NUM_LAST_TRACK = 4;
const int REUSE_MIN_NUM_LAST_TRACK = 2;
const double REUSE_LAST_BETTER_THAN_RATIO = 0.5;
#else
const int REUSE_NUM_LAST_TRACK = 20;
const int REUSE_MIN_NUM_LAST_TRACK = 10;
const double REUSE_LAST_BETTER_THAN_RATIO = 0.9;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NEW_STATE_NUM_LAST_TRACK = 4;
const int NEW_STATE_MIN_NUM_LAST_TRACK = 2;
const double NEW_STATE_LAST_BETTER_THAN_RATIO = 0.5;
#else
const int NEW_STATE_NUM_LAST_TRACK = 20;
const int NEW_STATE_MIN_NUM_LAST_TRACK = 10;
const double NEW_STATE_LAST_BETTER_THAN_RATIO = 0.9;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int UPDATE_NUM_ITERS = 40;
#else
const int UPDATE_NUM_ITERS = 400000;
// const int UPDATE_NUM_ITERS = 1000000;
#endif /* MDEBUG */

const int GENERALIZE_ITER = 3;

const int EXPERIMENT_REFRESH_NUM_ITERS = 10;

const int STUCK_NUM_ITERS = 10;

#endif /* CONSTANTS_H */