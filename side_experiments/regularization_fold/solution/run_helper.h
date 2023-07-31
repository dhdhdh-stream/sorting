#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	double predicted_score;
	double scale_factor;

	int curr_depth;	// need to track separate from context as context resets for experiments
	int max_depth;
	bool exceeded_depth;

	int explore_phase;
	bool can_random_iter;	// if EXPLORE_PHASE_UPDATE

	/**
	 * - for loops, choose explore iter on start so there's broader representation
	 * 
	 * - if explore seen but not triggered, change to EXPLORE_PHASE_UPDATE
	 */
	bool is_explore_iter;
	bool explore_seen;

	// class IDs to values
	std::map<int, double> last_seen_vals;

	AbstractExperiment* experiment;
	bool can_zero;
	std::vector<double> new_state_vals;
	bool experiment_on_path;
	int experiment_context_index;
	int experiment_step_index;

	int experiment_context_start_index;
	std::vector<int> experiment_helper_scope_context;
	std::vector<int> experiment_helper_node_context;
	/**
	 * - save separate copy of scope/node context for scope_node_additions_needed
	 *   - actual context may need to be empty to control branching
	 */

	double target_val;
	double final_misguess;

	std::vector<double> new_state_errors;
	std::vector<std::vector<double>> new_input_errors;

	bool backprop_is_pre_experiment;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->is_explore_iter = true;
		this->explore_seen = false;

		// initialize to false to simplify a check in Scope
		this->experiment_on_path = false;
	}
};

#endif /* RUN_HELPER_H */