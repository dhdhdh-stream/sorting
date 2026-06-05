#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int NETWORK_EPOCH_SIZE = 20;

const int STARTING_NUM_STATE = 4;
const int NUM_STATE_CHANGE = 4;

const int SAMPLES_NUM_SAVE = 10000;

#if defined(MDEBUG) && MDEBUG
const int SCORE_HISTORIES_NUM_SAVE = 100;
#else
const int SCORE_HISTORIES_NUM_SAVE = 4000;
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
// const int RAMP_EPOCH_NUM_ITERS = 2000;
// const int MEASURE_STEP_NUM_ITERS = 2000;
// const int TOTAL_MAX_ITERS = 100000;
const int RAMP_EPOCH_NUM_ITERS = 200;
const int MEASURE_STEP_NUM_ITERS = 200;
const int TOTAL_MAX_ITERS = 10000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */