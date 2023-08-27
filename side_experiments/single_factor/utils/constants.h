#ifndef CONSTANTS_H
#define CONSTANTS_H

// TODO: don't do all the option select
// - if miss, just abandon

const int EXPLORE_PHASE_NONE = ;
const int EXPLORE_PHASE_EXPLORE_SETUP = ;	// train new score network
const int EXPLORE_PHASE_EXPLORE = ;
const int EXPLORE_PHASE_EXPERIMENT_EXISTING_INNER = ;
const int EXPLORE_PHASE_EXPERIMENT_EXISTING_POST = ;
const int EXPLORE_PHASE_EXPERIMENT_EXISTING_PRE = ;
const int EXPLORE_PHASE_EXPERIMENT_NEW = ;

const int RUN_PHASE_UPDATE_NONE = ;
const int RUN_PHASE_UPDATE_REMEASURE = ;

const int EXPLORE_STATE_SETUP = 0;
const int EXPLORE_STATE_EXPLORE = 1;

#endif /* CONSTANTS_H */