#ifndef CONSTANTS_H
#define CONSTANTS_H

const int RUN_PHASE_EXPLORE = 0;
const int RUN_PHASE_UPDATE = 1;

const int RUN_PHASE_NEW = 2;
/**
 * - in new sequence, so don't explore or update
 */

/**
 * - at least 1000 to help against overtraining
 */
const int NUM_DATAPOINTS = 1000;

const double MIN_SCORE_IMPACT = 0.05;

#endif /* CONSTANTS_H */