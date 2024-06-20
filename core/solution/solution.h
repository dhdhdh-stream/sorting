/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 * 
 * - as long as have freedom to fail, not much use for world modeling
 *   - use world modeling when cost of failure is high, and have to explore along paths where there isn't risk of catastrophic failure
 * 
 * TODO:
 * - is sensitive to initial conditions
 *   - commits to paths greedily/randomly
 *     - if better path is not simple and cannot be generalized to, then will not be found
 *       - needs diversity
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <map>
#include <vector>

class AbstractExperiment;
class Eval;
class InfoScope;
class Problem;
class Scope;

class Solution {
public:
	int timestamp;
	double average_score;
	int next_possible_new_scope_timestamp;

	int last_updated_scope_id;
	int last_new_scope_id;

	std::vector<Scope*> scopes;
	std::vector<InfoScope*> info_scopes;

	/**
	 * - update on measure
	 * 
	 * - set limit to 10*max_num_actions+10
	 */
	int max_num_actions;
	int num_actions_limit;

	// temp
	std::vector<int> score_type_counts;
	std::vector<double> score_type_impacts;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	Solution(Solution* original);
	~Solution();

	void init();
	void load(std::string path,
			  std::string name);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::string path,
			  std::string name);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */