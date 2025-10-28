#ifndef BRANCH_END_NODE_H
#define BRANCH_END_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class BranchNode;
class Network;
class Problem;
class Solution;
class SolutionWrapper;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_DATAPOINTS = 20;
const int UPDATE_NUM_DATAPOINTS = 20;
#else
const int INITIAL_NUM_DATAPOINTS = 4000;
const int UPDATE_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

const int MAX_NUM_DATAPOINTS = 4000;

class BranchEndNodeHistory;
class BranchEndNode : public AbstractNode {
public:
	Network* pre_network;
	Network* post_network;

	int branch_start_node_id;
	BranchNode* branch_start_node;

	std::vector<std::vector<double>> pre_histories;
	std::vector<std::vector<double>> post_histories;
	std::vector<double> target_val_histories;
	int state_iter;

	int next_node_id;
	AbstractNode* next_node;

	BranchEndNode();
	~BranchEndNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void update(double result,
				BranchEndNodeHistory* history,
				SolutionWrapper* wrapper);
	void backprop();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class BranchEndNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> pre_histories;
	std::vector<double> post_histories;

	double signal_sum_vals;
	int signal_sum_counts;

	BranchEndNodeHistory(BranchEndNode* node);
};

#endif /* BRANCH_END_NODE_H */