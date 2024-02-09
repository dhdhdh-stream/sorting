/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 * 
 * - world modeling is good for surviving
 *   - which, for humans, is needed to reach the frontiers of knowledge
 *     - so world modeling is a foundation for innovation for humans
 * 
 * - innovation is simply slow and costly
 *   - only way to speed things up is to cheat, i.e, to learn
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#if defined(MDEBUG) && MDEBUG
const int STARTING_NUM_DATAPOINTS = 10;
#else
const int STARTING_NUM_DATAPOINTS = 400;
#endif /* MDEBUG */

class Solution {
public:
	int timestamp;

	int scope_counter;
	std::map<int, Scope*> scopes;

	Scope* root;

	/**
	 * - max depth for run that concluded
	 *   - set limit to max_depth+10/1.2*max_depth
	 * 
	 * - update on success for verify
	 */
	int max_depth;
	int depth_limit;

	int curr_num_datapoints;

	OuterExperiment* outer_experiment;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

};

#endif /* SOLUTION_H */