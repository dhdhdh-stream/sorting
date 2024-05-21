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

const int EXPLORE_TYPE_SCORE = 0;
const int EXPLORE_TYPE_EVAL = 1;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 2000;
#endif /* MDEBUG */

class Solution {
public:
	int timestamp;
	double timestamp_score;

	std::vector<Scope*> scopes;
	std::vector<InfoScope*> info_scopes;

	/**
	 * - final overall fallback to prevent infinite loops
	 *   - mainly prevent locally through experiments
	 * 
	 * - update on measure
	 * 
	 * - set limit to 20*max_num_actions+20
	 */
	int max_num_actions;
	int num_actions_limit;

	int explore_id;
	int explore_type;
	/**
	 * - measure on measure
	 */
	double explore_average_instances_per_run;
	int explore_scope_max_num_actions;
	double explore_scope_local_average_num_actions;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
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