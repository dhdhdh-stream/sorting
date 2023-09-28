/**
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
	/**
	 * - measure using sqr over abs
	 *   - even though sqr may not measure true score improvement, it measures information improvement
	 *     - which ultimately leads to better branching
	 */
	double average_misguess;
	double misguess_variance;

	int num_states;
	std::map<int, State*> score_states;
	/**
	 * - use map to track states as will be sparse
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
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	std::map<State*, StateStatus> score_state_snapshot;
	ObsExperimentHistory* obs_experiment_history;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */