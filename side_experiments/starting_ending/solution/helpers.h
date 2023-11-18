#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class BranchExperiment;
class BranchNode;
class PotentialScopeNode;
class Scope;
class ScopeHistory;
class ScopeNode;
class State;

void create_branch_experiment(ScopeHistory* root_history);
void create_pass_through_experiment(ScopeHistory* root_history);

PotentialScopeNode* create_scope(std::vector<ContextLayer>& context,
								 int explore_context_depth,
								 Scope* scope);

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

#endif /* HELPERS_H */