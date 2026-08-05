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

class InitNetwork;
class InitNetworkHistory;
class PassThroughNetwork;
class Problem;
class Scope;
class ScopeHistory;
class ScoreNetwork;
class Solution;
class SolutionWrapper;
class TransitionNetwork;
class TransitionNetworkHistory;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	std::vector<PassThroughNetwork*> in_pass_through_networks;
	TransitionNetwork* in_network;

	Scope* scope;

	std::vector<PassThroughNetwork*> out_pass_through_networks;
	TransitionNetwork* out_network;

	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_hit;
	double average_instances_per_run;
	AbstractExperiment* experiment;

	int curr_num_instances;

	ScopeNode();
	~ScopeNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void exit_step(std::vector<double>& obs,
				   SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);

	void train_step(AbstractNodeHistory* history,
					bool allow_drop,
					Eigen::VectorXf& state,
					TrainScopeHistory* train_scope_history);

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
	bool in_is_drop;
	TransitionNetworkHistory* in_network_history;

	TrainScopeHistory* scope_history;

	bool out_is_drop;
	TransitionNetworkHistory* out_network_history;

	TrainScopeNodeHistory(ScopeNode* node);
	~TrainScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */