#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class ContextHistory {
public:
	int scope_id;
	int node_id;

	double obs_snapshot;
	std::vector<double> input_vals_snapshot;
	std::vector<double> local_state_vals_snapshot;
};

class RunHelper {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	int explore_phase;
	double existing_score;
	double score_variance;

	FoldHistory* explore_fold_history;

	// to detect recursive calls for flat -- not fullproof but should be effective enough
	void* explore_location;
	bool is_recursive;

};

#endif /* RUN_HELPER_H */