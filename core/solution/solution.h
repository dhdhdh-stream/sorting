/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 * 
 * - as long as have freedom to fail, not much use for world modeling
 *   - use world modeling when cost of failure is high, and have to explore along paths where there isn't risk of catastrophic failure
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
	double score_standard_deviation;

	std::vector<Scope*> scopes;
	std::vector<InfoScope*> info_scopes;

	/**
	 * - update on measure
	 * 
	 * - set limit to 20*max_num_actions+20
	 */
	int max_num_actions;
	int num_actions_limit;

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

	// TODO: also add for info_scopes
	void clean_node(int scope_id,
					int node_id);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */