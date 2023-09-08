#ifndef RUN_HELPER_H
#define RUN_HELPER_H

const int REMEASURE_TYPE_BRANCH = 0;
const int REMEASURE_TYPE_LOOP = 1;

class RunHelper {
public:
	int phase;

	double running_average_score;
	double running_average_misguess;
	double predicted_score;
	double scale_factor;

	int curr_depth;	// need to track separate from context as context resets for experiments
	int max_depth;
	bool exceeded_depth;

	/**
	 * - for loops, choose explore iter on start so there's broader representation
	 */
	bool is_explore_iter;
	bool explore_seen;

	ExploreHistory* explore_history;

	int remeasure_type;
	RemeasureBranchNodeHistory* remeasure_branch_node_history;
	RemeasureScopeHistory* remeasure_scope_history;

	double target_val;
	double final_misguess;

};

#endif /* RUN_HELPER_H */