#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;

class RunHelper {
public:
	bool early_exit;

	int num_actions;

	AbstractExperimentHistory* experiment_history;

	bool verify_keypoints;
	std::vector<double> keypoint_misguess_factors;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */