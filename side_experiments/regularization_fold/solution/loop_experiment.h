/**
 * - on loop reuse (and occasionally), try all iters to learn state (or update weights)
 *   - don't try to learn continuous relation between state+iters and score
 *     - ineffective when relation is sharp
 *     - would still need to try all iters on reuse anyways
 *       - can't calculate iter gradient for iter equation with state being inaccurate
 *       - can't calculate state gradient for score equation with iter being inaccurate
 *         - and zigzag is unreliable
 */

#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

class LoopExperiment {
public:
	/**
	 * - for inputs, trace from matching outer scope
	 *   - keep (i.e., initialize) with 75% probability at explore(?)
	 *     - if kept at explore, then simply always include in new scope
	 */
	Sequence* sequence;
	Scale* scale_mod;

	ScoreNetwork* continue_score_network;
	ScoreNetwork* continue_misguess_network;
	ScoreNetwork* halt_score_network;
	ScoreNetwork* halt_misguess_network;



};

#endif /* LOOP_EXPERIMENT_H */