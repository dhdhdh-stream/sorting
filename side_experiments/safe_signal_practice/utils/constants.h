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

/**
 * - always give raw actions a large weight
 *   - existing scopes often learned to avoid certain patterns
 *     - which can prevent innovation
 */
const int RAW_ACTION_WEIGHT = 10;

#if defined(MDEBUG) && MDEBUG
const int HISTORIES_NUM_SAVE = 100;
#else
const int HISTORIES_NUM_SAVE = 4000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 5;
const int MEASURE_GEAR = 2;
const int RAMP_EPOCH_NUM_ITERS = 40;
const int MEASURE_STEP_NUM_ITERS = 40;
#else
const int EXPERIMENT_NUM_GEARS = 9;
const int MEASURE_GEAR = 4;
const int RAMP_EPOCH_NUM_ITERS = 4000;
const int MEASURE_STEP_NUM_ITERS = 4000;
#endif /* MDEBUG */

const int NUM_LAST_TRACK = 10;
#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_LAST_TRACK = 1;
#else
const int MIN_NUM_LAST_TRACK = 3;
#endif /* MDEBUG */
const double LAST_BETTER_THAN_RATIO = 0.6;

#endif /* CONSTANTS_H */