#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	int state_counter;
	std::map<int, State*> states;
	/**
	 * - use map to track states as will be sparse
	 */
	/**
	 * TODO: track relations among states, e.g.:
	 *   - which states are used together
	 *   - which states depend on which states
	 */

	int scope_counter;
	std::map<int, Scope*> scopes;
	// 0 is root, starts with ACTION_START

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	Solution(std::ifstream& input_file);
	~Solution();

	void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */