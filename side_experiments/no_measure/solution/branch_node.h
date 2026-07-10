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

const int MAINTAIN_NUM_ITERS = 20000;

const int CONSEC_DEPRECATE_LIMIT = 4000;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<Network*> original_networks;
	std::vector<Network*> branch_networks;
	std::vector<int> maintain_iters;
	/**
	 * - on update, only update last layer/iters
	 *   - so previous is an exact snapshot of when change happened
	 */

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	int ramp;
	int ramp_num_gears;
	int ramp_iter;

	int consec_original;
	int consec_branch;

	AbstractExperiment* original_experiment;
	AbstractExperiment* branch_experiment;

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

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	std::vector<double> obs;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */