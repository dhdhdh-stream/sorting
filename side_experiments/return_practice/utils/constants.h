#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int NETWORK_EPOCH_SIZE = 20;

const int STARTING_NUM_STATE = 4;

#if defined(MDEBUG) && MDEBUG
const int STATE_NUM_SAVE = 100;
#else
const int STATE_NUM_SAVE = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int SAMPLES_NUM_SAVE = 100;
#else
const int SAMPLES_NUM_SAVE = 10000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int SCORE_HISTORIES_NUM_SAVE = 100;
#else
const int SCORE_HISTORIES_NUM_SAVE = 1000;
#endif /* MDEBUG */

const int NUM_LAST_TRACK = 20;
#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_LAST_TRACK = 1;
#else
const int MIN_NUM_LAST_TRACK = 5;
#endif /* MDEBUG */
const double LAST_BETTER_THAN_RATIO = 0.8;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 3;
const int MEASURE_GEAR = 1;
const int RAMP_EPOCH_NUM_ITERS = 40;
const int MEASURE_STEP_NUM_ITERS = 40;
const int TOTAL_MAX_ITERS = 1000;
#else
const int EXPERIMENT_NUM_GEARS = 3;
const int MEASURE_GEAR = 1;
const int RAMP_EPOCH_NUM_ITERS = 100;
const int MEASURE_STEP_NUM_ITERS = 100;
const int TOTAL_MAX_ITERS = 10000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */