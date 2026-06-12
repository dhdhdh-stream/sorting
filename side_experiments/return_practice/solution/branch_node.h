#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"

class AbstractExperiment;
class Network;
class Wrapper;

class BranchNode : public AbstractNode {
public:
	Network* original_network;
	Network* branch_network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<std::vector<double>> original_state_history;
	std::vector<double> original_target_val_history;
	int original_history_index;
	AbstractExperiment* original_experiment;
	std::vector<std::vector<double>> branch_state_history;
	std::vector<double> branch_target_val_history;
	int branch_history_index;
	AbstractExperiment* branch_experiment;

	BranchNode();
	~BranchNode();

	void step(int& action,
			  bool& is_next,
			  Run* run);

	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void experiment_step_start(ExperimentRun* run);

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
	std::vector<double> large_state;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */