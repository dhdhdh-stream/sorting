#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"
#include "transformation.h"

class Scale;
class Scope;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_indexes;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	std::vector<bool> input_has_transform;
	std::vector<Transformation> input_transformations;

	Scale* scope_scale_mod;

	int next_node_id;

	ScopeNode(Scope* parent,
			  int id,
			  int inner_scope_id,
			  std::vector<int> starting_node_ids,
			  std::vector<int> input_indexes,
			  std::vector<int> input_target_layers,
			  std::vector<int> input_target_indexes,
			  std::vector<bool> input_has_transform,
			  std::vector<Transformation> input_transformations,
			  Scale* scope_scale_mod,
			  int next_node_id);
	ScopeNode(ScopeNode* original,
			  Scope* parent,
			  int id,
			  int next_node_id);
	ScopeNode(std::ifstream& input_file,
			  Scope* parent,
			  int id);
	~ScopeNode();

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  int& inner_exit_depth,
				  int& inner_exit_node_id,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void halfway_activate(std::vector<int>& starting_node_ids,
						  std::vector<std::vector<double>*>& starting_state_vals,
						  std::vector<std::vector<bool>>& starting_states_initialized,
						  std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  int& inner_exit_depth,
						  int& inner_exit_node_id,
						  RunHelper& run_helper,
						  ScopeNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void halfway_backprop(std::vector<int>& starting_node_ids,
						  std::vector<std::vector<double>*>& starting_state_errors,
						  std::vector<BackwardContextLayer>& context,
						  double& scale_factor_error,
						  RunHelper& run_helper,
						  ScopeNodeHistory* history);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory;
class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */