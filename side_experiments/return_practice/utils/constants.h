#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int SAMPLES_PER_TRAIN = 4000;

#if defined(MDEBUG) && MDEBUG
const int SCORE_HISTORIES_NUM_SAVE = 100;
#else
const int SCORE_HISTORIES_NUM_SAVE = 4000;
#endif /* MDEBUG */

const int STATE_HISTORY_NUM_SAVE = 4000;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_MIN_NUM_STATE_HISTORY = 10;
#else
const int EXPERIMENT_MIN_NUM_STATE_HISTORY = 1000;
#endif /* MDEBUG */

const int NUM_LAST_TRACK = 10;
#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_LAST_TRACK = 1;
#else
const int MIN_NUM_LAST_TRACK = 3;
#endif /* MDEBUG */
const double LAST_BETTER_THAN_RATIO = 0.6;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 5;
const int MEASURE_GEAR = 2;
const int RAMP_EPOCH_NUM_ITERS = 40;
const int MEASURE_STEP_NUM_ITERS = 40;
const int TOTAL_MAX_ITERS = 1000;
#else
const int EXPERIMENT_NUM_GEARS = 9;
const int MEASURE_GEAR = 4;
const int RAMP_EPOCH_NUM_ITERS = 2000;
const int MEASURE_STEP_NUM_ITERS = 2000;
const int TOTAL_MAX_ITERS = 100000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */