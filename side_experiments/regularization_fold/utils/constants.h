#ifndef CONSTANTS_H
#define CONSTANTS_H

const int EXPLORE_PHASE_UPDATE = -1;
const int EXPLORE_PHASE_NONE = 0;
const int EXPLORE_PHASE_EXPLORE = 1;
const int EXPLORE_PHASE_EXPERIMENT = 2;
const int EXPLORE_PHASE_CLEANUP = 3;
/**
 * - like UPDATE, but marks that went through experiment
 *   - check experiment_history to know which experiment was used
 */

#endif /* CONSTANTS_H */