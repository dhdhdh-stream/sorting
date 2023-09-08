#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	double average_score;
	double average_misguess;

	/**
	 * - max depth for run that didn't exceed
	 *   - set limit to max_depth+10/1.2*max_depth
	 */
	int max_depth;
	int depth_limit;

	/**
	 * - max decisions seen in single run
	 *   - use to determine how often branches/loops should re-measure
	 *     - set target to 1.5*max_decisions;
	 */
	int max_decisions;
	int remeasure_target;



};

#endif /* SOLUTION_H */