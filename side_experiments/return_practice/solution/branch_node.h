#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <vector>

#include "abstract_node.h"

class AbstractExperiment;
class BranchNetwork;
class Wrapper;

class BranchNode : public AbstractNode {
public:
	/**
	 * - need to crunch world modeling down into simple equation
	 *   - otherwise, expensive when many branches
	 */
	BranchNetwork* original_network;
	BranchNetwork* branch_network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	AbstractExperiment* original_experiment;
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