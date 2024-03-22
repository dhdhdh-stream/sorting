/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <map>
#include <vector>

class Problem;
class Scope;

#if defined(MDEBUG) && MDEBUG
const int STARTING_NUM_DATAPOINTS = 10;
#else
const int STARTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

const int TEMP_THROW_ID = -2;

class Solution {
public:
	int timestamp;

	int scope_counter;
	std::map<int, Scope*> scopes;

	Scope* root;

	int throw_counter;

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

	int curr_num_datapoints;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	~Solution();

	void init();
	void load(std::string path,
			  std::string name);

	void success_reset();
	void fail_reset();

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::string path,
			  std::string name);

	void remap();

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */