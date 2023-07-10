#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>

#include "scope.h"

class Solution {
public:
	double average_score;
	double average_misguess;

	std::vector<Scope*> scopes;
	// scopes[0] is root, starts with ACTION_START

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;


	/**
	 * TODO: explore based on current context
	 * - higher likelihood to start from closer scopes
	 */

}