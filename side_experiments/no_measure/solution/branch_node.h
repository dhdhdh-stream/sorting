/**
 * - don't have paths specifically for explore
 *   - i.e., don't separately optimize for explore and eval
 *     - can easily destroy progress for each other
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class ScopeHistory;
class Solution;
class SolutionWrapper;

const int RAMP_HISTORY_NUM_SAVE = 100;

const int CONSEC_DEPRECATE_LIMIT = 1000;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	Network* original_network;
	Network* branch_network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	int ramp;
	int ramp_iter;
	/**
	 * - need to save samples to increase training speed
	 *   - multiplying error on single samples washes away weights(?)
	 * 
	 * - need to increase training speed because initial networks don't match reality
	 */
	std::vector<std::vector<double>> original_obs_history;
	std::vector<double> original_target_val_history;
	int original_index;
	std::vector<std::vector<double>> branch_obs_history;
	std::vector<double> branch_target_val_history;
	int branch_index;

	int consec_original;
	int consec_branch;

	BranchNode();
	~BranchNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(BranchNode* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */