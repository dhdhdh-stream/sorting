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
	std::vector<double> new_state_errors;
	int experiment_context_index;
	int experiment_step_index;
	bool experiment_on_path;
	// helpers
	vector<vector<StateNetwork*>>* scope_state_networks;
	vector<ScoreNetwork*>* scope_score_networks;

	// to detect recursive calls for experiment -- not fullproof but hopefully effective enough
	int experiment_scope_id;
	bool is_recursive;

	double target_val;
	double final_misguess;

};

#endif /* RUN_HELPER_H */