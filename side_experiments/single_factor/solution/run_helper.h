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
	 * 
	 * - if explore seen but not triggered, change to RUN_PHASE_UPDATE_NONE
	 */
	bool is_explore_iter;
	bool explore_seen;

	double scale_factor_snapshot;	// for both explore setup and remeasure

	double explore_starting_running_average_score;
	ScoreNetworkHistory* explore_score_network_history;
	double explore_score_network_output;
	double explore_starting_running_average_misguess;
	ScoreNetworkHistory* explore_misguess_network_history;
	double explore_misguess_network_output;

	AbstractNode* explore_node;
	AbstractExperiment* explore_experiment;
	double explore_existing_score;

	int remeasure_type;
	BranchNodeHistory* remeasure_branch_node_history;
	ScopeHistory* remeasure_scope_history;

	double target_val;
	double final_misguess;

};

#endif /* RUN_HELPER_H */