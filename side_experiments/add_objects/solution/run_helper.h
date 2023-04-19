#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	int explore_phase;
	double existing_score;
	double score_variance;

	// to detect recursive calls for flat -- not fullproof but should be effective enough
	void* explore_location;
	bool is_recursive;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->explore_location = NULL;
		this->is_recursive = false;
	};
};

#endif /* RUN_HELPER_H */