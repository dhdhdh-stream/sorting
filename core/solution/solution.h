#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <map>
#include <set>
#include <vector>

#include "problem.h"

class AbstractExperiment;
class AbstractNode;
class OuterExperiment;
class Scope;
class State;

// const int STARTING_NUM_DATAPOINTS = 2000;
const int STARTING_NUM_DATAPOINTS = 10;

class Solution {
public:
	int state_counter;
	std::map<int, State*> states;
	/**
	 * - use map to track states as may be sparse
	 */
	/**
	 * TODO: track relations among states, e.g.:
	 *   - which states are used together
	 *   - which states depend on which states
	 */

	int scope_counter;
	std::map<int, Scope*> scopes;

	Scope* root;

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	int curr_num_datapoints;

	OuterExperiment* outer_experiment;
	std::set<AbstractExperiment*> experiments;

	void* verify_key;
	std::vector<Problem> verify_problems;

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void success_reset();
	void fail_reset();

	void clear_verify();

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */