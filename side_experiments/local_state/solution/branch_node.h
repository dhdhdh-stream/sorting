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

class InitNetwork;
class Problem;
class ScopeHistory;
class ScoreNetwork;
class ScoreNetworkHistory;
class Solution;
class SolutionWrapper;

const int CONSEC_DEPRECATE_LIMIT = 4000;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	ScoreNetwork* original_network;
	ScoreNetwork* branch_network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	bool is_ramp;

	int consec_original;
	int consec_branch;

	double original_average_instances_per_hit;
	double original_average_instances_per_run;
	AbstractExperiment* original_experiment;
	double branch_average_instances_per_hit;
	double branch_average_instances_per_run;
	AbstractExperiment* branch_experiment;

	int original_curr_num_instances;
	int branch_curr_num_instances;

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

	void train_step(AbstractNodeHistory* history,
					bool allow_drop,
					Eigen::VectorXf& state,
					TrainScopeHistory* train_scope_history);

	void copy_from(BranchNode* original,
				   Solution* parent_solution);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

class TrainBranchNodeHistory : public TrainAbstractNodeHistory {
public:
	bool is_branch;

	ScoreNetworkHistory* score_network_history;

	TrainBranchNodeHistory(BranchNode* node);
	~TrainBranchNodeHistory();
};

#endif /* BRANCH_NODE_H */