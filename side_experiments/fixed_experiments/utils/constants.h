#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
const std::vector<int> TRAIN_NEW_NUM_DATAPOINTS{10, 20, 40};
const int MEASURE_NUM_DATAPOINTS = 10;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 4000;
const std::vector<int> TRAIN_NEW_NUM_DATAPOINTS{100, 500, 4000};
/**
 * - good to have large amount of samples
 *   - update can save bad initial networks but requires many samples
 *     - 4000/40000 better than 100/400000
 */
const int MEASURE_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_ITERS = 30;
const std::vector<int> TRAIN_NEW_ITERS{30, 30, 30};
#else
const int TRAIN_EXISTING_ITERS = 300000;
const std::vector<int> TRAIN_NEW_ITERS{100000, 300000, 300000};
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
const int DIVERSITY_RANGE = 2;
#else
const int DIVERSITY_RANGE = 10;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_LAST_TRACK = 4;
const int MIN_NUM_LAST_TRACK = 2;
const double LAST_BETTER_THAN_RATIO = 0.5;
#else
const int NUM_LAST_TRACK = 10;
const int MIN_NUM_LAST_TRACK = 4;
const double LAST_BETTER_THAN_RATIO = 0.5;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int UPDATE_NUM_ITERS = 40;
const int BATCH_SIZE = 10;
const int ITERS_PER_BATCH = 10;
#else
const int UPDATE_NUM_ITERS = 40000;
const int BATCH_SIZE = 1000;
const int ITERS_PER_BATCH = 10000;
/**
 * - need large BATCH_SIZE and low ITERS_PER_BATCH
 *   - adam easily overfits
 */
#endif /* MDEBUG */

const int GENERALIZE_ITER = 3;

const int EXPERIMENT_REFRESH_NUM_ITERS = 10;

const int STUCK_NUM_ITERS = 12;

#endif /* CONSTANTS_H */