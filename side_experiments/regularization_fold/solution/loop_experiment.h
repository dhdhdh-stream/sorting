/**
 * - on loop reuse (and occasionally), try all iters to learn state (or weights)
 *   - don't try to learn continuous relation between state+iters and score
 *     - ineffective when relation is sharp
 *     - would still need to try all iters on reuse anyways, as can't calculate accurate gradient off single iter
 */

#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

class LoopExperiment {
public:
	
};

#endif /* LOOP_EXPERIMENT_H */