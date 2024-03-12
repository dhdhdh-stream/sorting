#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class Scope;
class ScopeHistory;
class ScopeNode;
class SeedExperimentFilter;

void create_experiment(ScopeHistory* root_history);
AbstractExperiment* create_experiment(std::ifstream& input_file);

ScopeNode* reuse_existing();
ScopeNode* create_scope(Scope* parent_scope,
						RunHelper& run_helper);
ScopeNode* create_repeat(std::vector<ContextLayer>& context,
						 int explore_context_depth);

void gather_possible_exits(std::vector<std::pair<int,AbstractNode*>>& possible_exits,
						   std::vector<Scope*>& experiment_scope_context,
						   std::vector<AbstractNode*>& experiment_node_context,
						   bool experiment_is_branch,
						   int experiment_throw_id,
						   RunHelper& run_helper);

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							bool on_path,
							int path_depth,
							std::vector<int>& context_match_indexes,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							std::vector<int>& possible_strict_root_indexes,
							ScopeHistory* scope_history);

void create_gather(std::vector<Scope*>& new_gather_scope_context,
				   std::vector<AbstractNode*>& new_gather_node_context,
				   bool& new_gather_is_branch,
				   int& new_gather_exit_depth,
				   AbstractNode*& new_gather_exit_node,
				   ScopeHistory* scope_history,
				   SeedExperimentFilter* seed_experiment_filter);

/**
 * TODO:
 * - instead of always backtracking, add caching?
 */
void input_vals_helper(std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   bool on_path,
					   int path_depth,
					   std::vector<int>& context_match_indexes,
					   std::vector<double>& input_vals,
					   ScopeHistory* scope_history);

#endif /* SOLUTION_HELPERS_H */