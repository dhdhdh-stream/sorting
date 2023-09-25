/**
 * - scopes are just a bit of abstraction to try to promote the reuse of ideas
 *   - scopes roughly capture when certain state is relevant, but is extremely imprecise
 *     - the actual corresponding actions may start sooner or later, and may end sooner or later
 *   - in addition, may need to constantly break up scopes to use elsewhere
 *   - but as long as scopes, inner scopes, etc., are being created and reused, should be good enough to make progress
 * 
 * - specifically, here, the states created are too loose
 *   - i.e., they likely include more actions than is relevant for the state
 *     - so from the outside, there's more that's abstracted, but from the inside, will miss possible sub-scopes
 *       - though sub-scopes will likely be created on branch
 *         - so again, probably good enough
 * 
 * TODO: to handle loops, average values, and possibly select one iter/context to focus on
 * - even if only one iter is relevant, and that iter changes, will still be correlation
 */

#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_input_states;
	int num_local_states;

	/**
	 * - no need to explicitly track score states here
	 */

	std::vector<AbstractNode*> nodes;

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	int num_score_states;
	std::map<int, State*> score_states;
	/**
	 * - use map to track score_states as will be sparse
	 */

	std::map<int, State*> states;
	/**
	 * TODO: track relations among states, e.g.:
	 *   - which states are used together
	 *   - which states depend on which states
	 */

	std::vector<Scope*> child_scopes;
	/**
	 * - for constructing new sequences
	 */

	ObsExperiment* obs_experiment;



};

class ScopeHistory {
public:
	
};

#endif /* SCOPE_H */