#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class ClassDefinition;
class FamilyDefinition;
class Scope;

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
	/**
	 * - constantly update misguess_standard_deviation for efficiency
	 *   - but not for score as only used on experiment transform
	 */
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

	Solution();
	Solution(std::ifstream& input_file);
	~Solution();

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */