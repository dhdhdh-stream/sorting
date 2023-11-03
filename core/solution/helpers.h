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

void create_branch_experiment(ScopeHistory* root_history);

Sequence* create_sequence(Problem& problem,
						  std::vector<ContextLayer>& context,
						  int explore_context_depth,
						  Scope* containing_scope,
						  RunHelper& run_helper);

void add_state(Scope* parent_scope,
			   int temp_state_index,
			   std::vector<int>& experiment_scope_context,
			   std::vector<int>& experiment_node_context,
			   int outer_scope_depth,
			   std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings);
ScopeNode* finalize_sequence(std::vector<int>& experiment_scope_context,
							 std::vector<int>& experiment_node_context,
							 Sequence* new_sequence,
							 std::map<std::pair<int, std::pair<bool,int>>, int>& input_scope_depths_mappings,
							 std::map<std::pair<int, std::pair<bool,int>>, int>& output_scope_depths_mappings);

#endif /* HELPERS_H */