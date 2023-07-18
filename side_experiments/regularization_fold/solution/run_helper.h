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
	// TODO: choose iter on the outside from loop
	// if explore exists, but not triggered, still track

	std::map<StateDefinition*, double> last_seen_vals;

	AbstractExperiment* experiment;
	bool can_zero;
	std::vector<double> new_state_vals;
	bool experiment_on_path;
	int experiment_context_index;
	int experiment_step_index;
	std::vector<int> experiment_off_path_scope_context;
	std::vector<int> experiment_off_path_node_context;

	// to detect recursive calls for experiment -- not fullproof but hopefully effective enough
	int experiment_scope_id;
	bool is_recursive;

	double target_val;
	double final_misguess;

	std::vector<double> new_state_errors;
	std::vector<std::vector<double>> new_input_errors;

	bool backprop_is_pre_experiment;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		// initialize to false to make some Scope logic slightly easier
		this->experiment_on_path = false;

		this->experiment_scope_id = -1;
		this->is_recursive = false;
	}
};

#endif /* RUN_HELPER_H */