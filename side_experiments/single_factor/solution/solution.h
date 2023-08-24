#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:


	/**
	 * - max depth for run that didn't exceed
	 *   - set limit to max_depth+10/1.2*max_depth
	 */
	int max_depth;
	int depth_limit;

	/**
	 * - max decisions seen in single run
	 *   - use to determine how often branches/loops should re-measure
	 */
	int max_decisions;
};

#endif /* SOLUTION_H */