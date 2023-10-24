#ifndef CONSTANTS_H
#define CONSTANTS_H

const int RUN_PHASE_EXPLORE = 0;
const int RUN_PHASE_UPDATE = 1;

/**
 * - for if simple_activate(), don't experiment within
 */
const int RUN_PHASE_NEW = 2;

const int NUM_DATAPOINTS = 2000;

const int MEASURE_ITERS = 2000;

const double MIN_SCORE_IMPACT = 0.05;

#endif /* CONSTANTS_H */