/**
 * - predict directly against final result
 *   - don't try to attribute impact to each part of solution
 *     - setup and actual execution equally responsible for success
 *     - subtracting many signals from true instable
 * 
 * - need big network
 *   - need to predict parts that are not actionable to "filter" them out
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <fstream>
#include <map>
#include <vector>

#include "signal_input.h"

class ScopeHistory;
class SignalNode;
class Solution;

#if defined(MDEBUG) && MDEBUG
const int SIGNAL_NUM_SAMPLES = 50;
#else
const int SIGNAL_NUM_SAMPLES = 50000;
#endif /* MDEBUG */

const int POTENTIAL_NUM_TRIES = 10;
const int SIGNAL_NODE_MAX_NUM_INPUTS = 20;

class Signal {
public:
	std::vector<SignalNode*> nodes;

	std::vector<double> output_weights;
	double output_constant;
	std::vector<double> output_weight_updates;
	double output_constant_update;

	int epoch_iter;
	double average_max_update;

	std::vector<std::vector<std::vector<double>>> input_val_histories;
	std::vector<std::vector<std::vector<bool>>> input_is_on_histories;
	std::vector<double> target_val_histories;
	int history_index;

	std::vector<std::vector<SignalInput>> potential_inputs;
	std::vector<std::vector<std::vector<double>>> potential_input_val_histories;
	std::vector<std::vector<std::vector<bool>>> potential_input_is_on_histories;
	int potential_count;

	Signal();
	~Signal();

	double activate(ScopeHistory* scope_history,
					std::vector<int>& explore_index);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);

	void copy_from(Signal* original,
				   Solution* parent_solution);

	double activate_helper(int index);
	void backprop_helper(int index);
};

#endif /* SIGNAL_H */