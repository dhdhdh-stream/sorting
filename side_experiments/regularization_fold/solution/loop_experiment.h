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
	// find matching outer scope, and trace inputs inwards to reuse
	// - for last seen, reuse with like 75% probability?
	Sequence* sequence;
	Scale* scale_mod;

	ScoreNetwork* continue_score_network;
	ScoreNetwork* continue_misguess_network;
	ScoreNetwork* halt_score_network;
	ScoreNetwork* halt_misguess_network;

};

#endif /* LOOP_EXPERIMENT_H */