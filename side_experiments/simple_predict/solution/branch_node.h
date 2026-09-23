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
class ScoreNetwork;
class ScoreNetworkHistory;
class Solution;
class SolutionWrapper;

const int CONSEC_DEPRECATE_LIMIT = 4000;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	Network* original_network;
	Network* branch_network;

	ScoreNetwork* branch_predict_network;
	/**
	 * - <-1.0 if original, >1.0 if branch
	 * - backprop errors to force state to capture how decision is made(?)
	 */

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	int consec_original;
	int consec_branch;

	double original_average_instances_per_hit;
	double original_average_instances_per_run;
	std::vector<AbstractExperiment*> original_experiments;
	double branch_average_instances_per_hit;
	double branch_average_instances_per_run;
	std::vector<AbstractExperiment*> branch_experiments;

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
					Eigen::VectorXf& state,
					bool& is_done,
					TrainScopeHistory* train_scope_history);
	void train_predict_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							TrainScopeHistory* train_scope_history);

	void predict_step(Eigen::VectorXf& state,
					  AbstractNode*& node_context);

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

	std::vector<double> obs;

	BranchNodeHistory(BranchNode* node);
};

class TrainPredictBranchNodeHistory : public TrainAbstractNodeHistory {
public:
	bool is_branch;
	ScoreNetworkHistory* predict_branch_network_history;

	TrainPredictBranchNodeHistory(BranchNode* branch_node);
	~TrainPredictBranchNodeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error);
};

#endif /* BRANCH_NODE_H */