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

	std::map<TypeDefinition*, double> last_seen_vals;
	// update at end of scope nodes

	AbstractExperiment* experiment;
	std::vector<double> new_state_vals;
	int experiment_context_index;
	bool can_zero;
	// helpers
	vector<vector<Network*>>* scope_state_networks;
	vector<Network*>* scope_score_networks;


	// to detect recursive calls for experiment -- not fullproof but hopefully effective enough
	int experiment_scope_id;
	bool is_recursive;
};

#endif /* RUN_HELPER_H */