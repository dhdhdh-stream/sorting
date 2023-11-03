#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <map>
#include <set>

class BranchExperiment;
class Scope;
class State;

const int STARTING_NUM_DATAPOINTS = 1000;

class Solution {
public:
	double average_score;

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

	int root_id;

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	/**
	 * - for least squares, ObsExperiment, and measure
	 * 
	 * - double every 20 failures
	 */
	int curr_num_datapoints;

	OuterExperiment* outer_experiment;
	std::set<AbstractExperiment*> experiments;

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */