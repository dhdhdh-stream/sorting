/**
 * - don't bother with hooks into inner
 *   - quite messy when multiple hooks on same node
 *     - if try one by one, cannot override early mistake
 *     - if only use one outer, restrictive
 *   - simply rely on inner's child scopes instead
 */

#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_node.h"

class PredictNetwork;
class PredictNetworkHistory;
class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	bool is_generic;

	Scope* scope;

	PredictNetwork* predict_network;

	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_hit;
	double average_instances_per_run;
	std::vector<AbstractExperiment*> experiments;

	int curr_num_instances;

	ScopeNode();
	~ScopeNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void exit_step(SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);

	void train_step(AbstractNodeHistory* history,
					Eigen::VectorXf& state,
					bool& hit_explore,
					TrainScopeHistory* train_scope_history);

	void predict_step(Eigen::VectorXf& state,
					  AbstractNode*& node_context);

	void copy_from(ScopeNode* original,
				   Solution* parent_solution);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	ScopeNodeHistory(ScopeNode* node);
	~ScopeNodeHistory();
};

class TrainScopeNodeHistory : public TrainAbstractNodeHistory {
public:
	TrainScopeHistory* scope_history;

	TrainScopeNodeHistory(ScopeNode* node);
	~TrainScopeNodeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error,
				  SolutionWrapper* wrapper);
	void update(int iter_index);
};

class TrainPredictScopeNodeHistory : public TrainAbstractNodeHistory {
public:
	PredictNetworkHistory* predict_network_history;

	TrainPredictScopeNodeHistory(ScopeNode* node);
	~TrainPredictScopeNodeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error,
				  SolutionWrapper* wrapper);
	void update(int iter_index);
};

#endif /* SCOPE_NODE_H */