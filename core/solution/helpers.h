#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class BranchNode;
class Scope;
class ScopeHistory;
class ScopeNode;
class Sequence;
class State;

void create_branch_experiment(ScopeHistory* root_history);
void create_pass_through_experiment(ScopeHistory* root_history);

Sequence* create_sequence(Problem& problem,
						  std::vector<ContextLayer>& context,
						  int explore_context_depth,
						  Scope* containing_scope,
						  RunHelper& run_helper);
ScopeNode* create_root_halfway_start(Problem& problem,
									 std::vector<ContextLayer>& context,
									 RunHelper& run_helper);

void existing_obs_experiment(AbstractExperiment* experiment,
							 Scope* parent_scope,
							 std::vector<ScopeHistory*>& scope_histories,
							 std::vector<double>& target_vals);
void new_obs_experiment(AbstractExperiment* experiment,
						std::vector<ScopeHistory*>& scope_histories,
						std::vector<double>& target_vals);

ScopeNode* finalize_sequence(std::vector<int>& experiment_scope_context,
							 std::vector<int>& experiment_node_context,
							 Sequence* new_sequence,
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