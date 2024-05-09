/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <map>
#include <vector>

class AbstractExperiment;
class Eval;
class InfoScope;
class NewActionTracker;
class Problem;
class Scope;

const int SOLUTION_STATE_TRAVERSE = 0;
const int SOLUTION_STATE_GENERALIZE = 1;
const int SOLUTION_STATE_EVAL = 2;

#if defined(MDEBUG) && MDEBUG
const int SOLUTION_TRAVERSE_ITERS = 5;
#else
const int SOLUTION_TRAVERSE_ITERS = 20;
#endif /* MDEBUG */

class Solution {
public:
	int timestamp;
	double curr_average_score;
	double average_num_actions;

	int state;
	int state_iter;

	int num_actions_until_experiment;
	int num_actions_until_random;

	Scope* current;
	std::vector<Scope*> scopes;
	std::vector<InfoScope*> info_scopes;

	Eval* eval;

	NewActionTracker* new_action_tracker;

	/**
	 * - max depth for run that concluded
	 *   - set limit to max_depth+10/1.2*max_depth
	 * 
	 * - update on success for verify
	 */
	int max_depth;
	int depth_limit;

	/**
	 * - set limit to 20*max_num_actions+20
	 */
	int max_num_actions;
	int num_actions_limit;

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

	void increment();

	void save(std::string path,
			  std::string name);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */