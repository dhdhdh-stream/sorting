#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int explore_phase;
	// TODO: choose iter on the outside from loop
	// if explore exists, but not triggered, still track

	Experiment* experiment;
	std::vector<double> new_state_vals;

	// to detect recursive calls for experiment -- not fullproof but hopefully effective enough
	int experiment_scope_id;
	bool is_recursive;
};

#endif /* RUN_HELPER_H */