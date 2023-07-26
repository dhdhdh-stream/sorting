#ifndef CONSTANTS_H
#define CONSTANTS_H

const int EXPLORE_PHASE_UPDATE = -1;
/**
 * - always have some proportion of EXPLORE_PHASE_UPDATE to balance out network updates from experiment WRAPUP
 */
const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_EXPLORE = 1;
const int EXPLORE_PHASE_EXPERIMENT = 2;
const int EXPLORE_PHASE_MEASURE = 3;
const int EXPLORE_PHASE_CLEAN = 4;
const int EXPLORE_PHASE_WRAPUP = 5;

#endif /* CONSTANTS_H */