#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - lean towards being more selective
 */
const double MATCH_WEIGHT = 0.5;

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SCOPE = 1;

const int LAST_NUM_TRACK = 1000;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_NUM_GEARS = 4;
const int EVAL_GEAR = 2;
const int RAMP_EPOCH_NUM_ITERS = 200;
#else
const int EXPERIMENT_NUM_GEARS = 10;
const int EVAL_GEAR = 5;
const int RAMP_EPOCH_NUM_ITERS = 2000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */