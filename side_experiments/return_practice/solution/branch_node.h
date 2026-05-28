#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"

class Experiment;
class Network;
class Wrapper;

class BranchNode : public AbstractNode {
public:
	/**
	 * - need to crunch world modeling down into simple equation
	 *   - otherwise, expensive when many branches
	 */
	Network* original_network;
	int original_network_epoch_iter;
	Network* branch_network;
	int branch_network_epoch_iter;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<std::vector<double>> original_state_history;
	int original_state_history_index;
	std::vector<std::vector<double>> branch_state_history;
	int branch_state_history_index;

	double original_average_instances_per_run;
	double branch_average_instances_per_run;

	Experiment* original_experiment;
	Experiment* branch_experiment;

	int original_sum_instances;
	int branch_sum_instances;

	BranchNode();
	~BranchNode();

	void step(int& action,
			  bool& is_next,
			  Run* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void predict_step(PredictRun* run);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file,
			  Wrapper* wrapper);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	std::vector<double> state;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */