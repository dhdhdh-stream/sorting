/**
 * - focus signals on consistency
 *   - if innovation occurs enough to be recognized, it will likely already have been added
 *   - so don't worry about explore samples being good/innovative
 *     - just capture a wide variety
 */

#ifndef BRANCH_END_NODE_H
#define BRANCH_END_NODE_H

#include <fstream>
#include <utility>
#include <vector>

#include "abstract_node.h"

class BranchNode;
class Network;
class Problem;
class Solution;
class SolutionWrapper;

#if defined(MDEBUG) && MDEBUG
const int MIN_NUM_DATAPOINTS = 10;
#else
const int MIN_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

const int MAX_NUM_DATAPOINTS = 4000;

class BranchEndNodeHistory;
class BranchEndNode : public AbstractNode {
public:
	Network* pre_network;
	Network* post_network;

	int branch_start_node_id;
	BranchNode* branch_start_node;

	std::vector<std::vector<double>> existing_pre_histories;
	std::vector<std::vector<double>> existing_post_histories;
	std::vector<double> existing_target_val_histories;
	// temp
	std::vector<std::pair<int, int>> start_locations;
	std::vector<std::pair<int, int>> end_locations;

	std::vector<std::vector<double>> explore_pre_histories;
	std::vector<std::vector<double>> explore_post_histories;
	std::vector<double> explore_target_val_histories;

	int next_node_id;
	AbstractNode* next_node;

	// temp
	std::vector<int> travel_counts;

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
	// temp
	std::pair<int, int> start_location;
	std::pair<int, int> end_location;

	double signal_sum_vals;
	int signal_sum_counts;

	BranchEndNodeHistory(BranchEndNode* node);
};

#endif /* BRANCH_END_NODE_H */