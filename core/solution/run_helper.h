#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <vector>

class Problem;
class ScopeHistory;
class Solution;

class RunHelper {
public:
	int num_decisions;
	int num_actions;

	/**
	 * - set by experiment when exploring/new
	 */
	int num_actions_limit;

	ScopeHistory* experiment_scope_history;

	Solution* success_duplicate;

	std::vector<double> predicted_scores;

	#if defined(MDEBUG) && MDEBUG
	unsigned long curr_run_seed;

	Problem* problem_snapshot;
	unsigned long run_seed_snapshot;

	void* verify_key;
	#endif /* MDEBUG */

	RunHelper();
};

#endif /* RUN_HELPER_H */