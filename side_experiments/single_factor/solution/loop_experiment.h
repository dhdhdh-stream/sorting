#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

const int LOOP_EXPERIMENT_STATE_EXPLORE = -1;

/**
 * - learn inputs and exits
 *   - for each input, it may be passed into the sequence twice:
 *     - copied, and passed in as input to use existing networks
 *     - updated with new networks
 */
const int LOOP_EXPERIMENT_STATE_EXISTING = 0;
/**
 * - multiple rounds to learn score
 *   - randomize number of iters
 *   - progress from inner to outer
 */
const int LOOP_EXPERIMENT_STATE_NEW = 1;
/**
 * - learn continue and halt networks
 */
const int LOOP_EXPERIMENT_STATE_LOOP = 2;
/**
 * - measure and modify solution if success
 */
const int LOOP_EXPERIMENT_STATE_DONE = 3;



#endif /* LOOP_EXPERIMENT_H */