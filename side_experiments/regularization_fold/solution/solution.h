#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>

#include "scope.h"

class Solution {
public:
	/**
	 * - just keep track of overall variances instead of per node
	 *   - may prevent incremental improvements on clear paths from being recognized, but reduces bookkeeping
	 */
	double average_score;
	double score_variance;
	/**
	 * - i.e., the portion of the score variance that cannot be explained away
	 *   - so minimal but significant value to compare against for branching
	 */
	double average_misguess;
	double misguess_variance;
	double misguess_standard_deviation;

	std::vector<Scope*> scopes;
	// scopes[0] is root, starts with ACTION_START
	std::vector<FamilyDefinition*> families;
	std::vector<ClassDefinition*> classes;

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;


	/**
	 * TODO: explore based on current context
	 * - higher likelihood to start from closer scopes
	 */

}