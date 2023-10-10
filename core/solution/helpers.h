#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class AbstractNode;
class BranchNode;
class ObsExperiment;
class Scope;
class ScopeHistory;
class ScopeNode;
class Sequence;
class State;

ObsExperiment* create_obs_experiment(ScopeHistory* scope_history);
ObsExperiment* create_decision_obs_experiment(ScopeHistory* scope_history);
ObsExperiment* create_full_obs_experiment(ScopeHistory* scope_history);

void create_branch_experiment(ScopeHistory* root_history);

Sequence* create_sequence(Problem& problem,
						  std::vector<ContextLayer>& context,
						  int explore_context_depth,
						  Scope* containing_scope,
						  RunHelper& run_helper);
Sequence* create_root_sequence(Problem& problem,
							   std::vector<ContextLayer>& context,
							   int explore_context_depth,
							   RunHelper& run_helper);

void random_exit(std::vector<int>& starting_scope_context,
				 std::vector<int>& starting_node_context,
				 int& new_exit_depth,
				 int& new_exit_node_id);

void finalize_existing_state(Scope* parent_scope,
							 State* score_state,
							 BranchNode* new_branch_node,
							 double new_branch_weight);
void finalize_new_state(Scope* parent_scope,
						std::map<int, Sequence*>& sequence_mappings,
						State* score_state,
						std::vector<AbstractNode*>& nodes,
						std::vector<std::vector<int>>& scope_contexts,
						std::vector<std::vector<int>>& node_contexts,
						std::vector<int>& obs_indexes,
						BranchNode* new_branch_node);
ScopeNode* finalize_sequence(std::vector<int>& scope_context,
							 std::vector<int>& node_context,
							 Sequence* new_sequence,
							 std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
							 std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);
void finalize_new_score_state(Scope* parent_scope,
							  std::map<int, int>& new_scope_node_id_mappings,
							  State* score_state,
							  std::vector<AbstractNode*>& nodes,
							  std::vector<std::vector<int>>& scope_contexts,
							  std::vector<std::vector<int>>& node_contexts,
							  std::vector<int>& obs_indexes);

#endif /* HELPERS_H */