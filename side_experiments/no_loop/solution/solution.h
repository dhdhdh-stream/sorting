#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	std::vector<Scope*> scopes;
	// scopes[0] is root, starts with ACTION_START

	int num_score_states;
	std::map<int, State*> score_states;
	/**
	 * - use map to track score_states as will be sparse
	 */

	std::map<int, State*> states;
	/**
	 * TODO: track relations among states, e.g.:
	 *   - which states are used together
	 *   - which states depend on which states
	 */

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;



	void random_start();


};

#endif /* SOLUTION_H */