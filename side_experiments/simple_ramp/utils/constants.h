#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

#if defined(MDEBUG) && MDEBUG
const int HISTORIES_NUM_SAVE = 100;
#else
const int HISTORIES_NUM_SAVE = 4000;
#endif /* MDEBUG */

const int LAST_NUM_TRACK = 1000;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 6;
const int RAMP_GEAR = 1;
const int MEASURE_GEAR = 2;
const int RAMP_EPOCH_NUM_ITERS = 20;
const int MEASURE_EPOCH_NUM_ITERS = 80;
#else
const int EXPERIMENT_NUM_GEARS = 10;
const int RAMP_GEAR = 2;
const int MEASURE_GEAR = 4;
const int RAMP_EPOCH_NUM_ITERS = 4000;
const int MEASURE_EPOCH_NUM_ITERS = 8000;
#endif /* MDEBUG */

const int BRANCH_RATIO_CHECK_ITER = 1000;
const double BRANCH_MIN_RATIO = 0.03;

/**
 * - track relatively large number
 *   - to help prevent string of bad luck from adding bad changes
 */
const int NUM_LAST_TRACK = 40;
const double LAST_BETTER_THAN_RATIO = 0.8;

#endif /* CONSTANTS_H */