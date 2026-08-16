#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const double SCORE_LEARNING_RATE = 0.001;
const double STATE_LEARNING_RATE = 0.0002;

const double NETWORK_INIT_MULTIPLIER = 0.01;

const int INIT_EPOCH_SIZE = 10;
const int UPDATE_EPOCH_SIZE = 100;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_TRAIN_NUM_DATAPOINTS = 20;
const int EXPERIMENT_MEASURE_NUM_DATAPOINTS = 10;
#else
const int EXPERIMENT_TRAIN_NUM_DATAPOINTS = 200;
const int EXPERIMENT_MEASURE_NUM_DATAPOINTS = 50;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 100000;
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
const int NUM_LAST_TRACK = 4;
const int MIN_NUM_LAST_TRACK = 2;
const double LAST_BETTER_THAN_RATIO = 0.5;
#else
const int NUM_LAST_TRACK = 10;
const int MIN_NUM_LAST_TRACK = 5;
const double LAST_BETTER_THAN_RATIO = 0.6;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int UPDATE_NUM_ITERS = 40;
const int BATCH_SIZE = 10;
const int ITERS_PER_BATCH = 10;
#else
const int UPDATE_NUM_ITERS = 10000;
const int BATCH_SIZE = 1000;
const int ITERS_PER_BATCH = 10000;
#endif /* MDEBUG */

const int RUN_TYPE_EXISTING = 0;
const int RUN_TYPE_EXPLORE = 1;

const int GENERALIZE_ITER = 3;

const int EXPERIMENT_REFRESH_NUM_ITERS = 10;

const int STUCK_NUM_ITERS = 10;

#endif /* CONSTANTS_H */