#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int phase;

	int curr_depth;	// need to track separate from context as context resets for experiments
	int max_depth;
	bool exceeded_depth;

	/**
	 * - don't need to track running tally of predicted score
	 *   - instead, on experiment, calculate from partial and resolved score_state_vals
	 */

	std::vector<ScopeHistory*> scope_histories;

	BranchExperimentHistory* branch_experiment_history;

};

#endif /* RUN_HELPER_H */