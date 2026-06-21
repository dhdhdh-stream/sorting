#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"

class AbstractExperiment;
class Network;
class Wrapper;

const int RAMP_TYPE_PREDICT = 0;
const int RAMP_TYPE_FORCE = 1;

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
	int original_iter;

	int ramp_type;

	double original_average_instances_per_run;
	std::vector<std::vector<double>> original_state_history;
	std::vector<double> original_target_val_history;
	int original_history_index;
	AbstractExperiment* original_experiment;
	double branch_average_instances_per_run;
	std::vector<std::vector<double>> branch_state_history;
	std::vector<double> branch_target_val_history;
	int branch_history_index;
	AbstractExperiment* branch_experiment;

	int original_curr_instances_per_run;
	int branch_curr_instances_per_run;

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
	bool would_be_branch;

	std::vector<double> state;

	int obs_history_index;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */