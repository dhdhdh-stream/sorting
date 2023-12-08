#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class BranchExperiment;
class BranchNode;
class PotentialScopeNode;
class Scope;
class ScopeHistory;
class ScopeNode;
class State;

void create_experiment(ScopeHistory* root_history);

PotentialScopeNode* create_scope(std::vector<ContextLayer>& context,
								 int explore_context_depth,
								 Scope* scope);
PotentialScopeNode* create_loop(std::vector<ContextLayer>& context,
								int explore_context_depth);

void gather_possible_exits(std::vector<std::pair<int,AbstractNode*>>& possible_exits,
						   std::vector<ContextLayer>& context,
						   std::vector<int>& scope_context,
						   std::vector<int>& node_context);
void parent_pass_through_gather_possible_exits(
	std::vector<std::pair<int,AbstractNode*>>& possible_exits,
	std::vector<ContextLayer>& context,
	std::vector<int>& scope_context,
	std::vector<int>& node_context,
	int parent_exit_depth,
	AbstractNode* parent_exit_node);

void existing_obs_experiment(AbstractExperiment* experiment,
							 Scope* parent_scope,
							 std::vector<ScopeHistory*>& scope_histories,
							 std::vector<double>& target_vals);
void new_obs_experiment(AbstractExperiment* experiment,
						std::vector<ScopeHistory*>& scope_histories,
						std::vector<double>& target_vals);
void existing_pass_through_branch_obs_experiment(
		BranchExperiment* experiment,
		std::vector<ScopeHistory*>& scope_histories,
		std::vector<double>& target_vals);

void finalize_potential_scope(std::vector<int>& experiment_scope_context,
							  std::vector<int>& experiment_node_context,
							  PotentialScopeNode* potential_scope_node,
							  std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
							  std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
void finalize_branch_node_states(BranchNode* branch_node,
								 std::vector<std::map<int, double>>& existing_input_state_weights,
								 std::vector<std::map<int, double>>& existing_local_state_weights,
								 std::vector<std::map<State*, double>>& existing_temp_state_weights,
								 std::vector<std::map<int, double>>& new_input_state_weights,
								 std::vector<std::map<int, double>>& new_local_state_weights,
								 std::vector<std::map<State*, double>>& new_temp_state_weights,
								 std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
								 std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
void finalize_loop_scope_node_states(ScopeNode* new_loop_scope_node,
									 std::vector<int>& experiment_scope_context,
									 std::vector<int>& experiment_node_context,
									 std::vector<std::map<int, double>>& existing_input_state_weights,
									 std::vector<std::map<int, double>>& existing_local_state_weights,
									 std::vector<std::map<State*, double>>& existing_temp_state_weights,
									 std::vector<std::map<int, double>>& new_input_state_weights,
									 std::vector<std::map<int, double>>& new_local_state_weights,
									 std::vector<std::map<State*, double>>& new_temp_state_weights,
									 std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
									 std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);

#endif /* HELPERS_H */