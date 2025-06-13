#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <set>
#include <utility>
#include <vector>

#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_NEW_SCOPE = 2;
/**
 * - no CommitExperiment as solution should already be large
 *   - and practically, likely too slow
 */

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	virtual ~AbstractExperiment() {};

	virtual void activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history) = 0;
	virtual void backprop(double target_val,
						  AbstractExperimentHistory* history,
						  std::set<Scope*>& updated_scopes) = 0;

	virtual void clean_inputs(Scope* scope,
							  int node_id) = 0;
	virtual void clean_inputs(Scope* scope) = 0;
	virtual void replace_factor(Scope* scope,
								int original_node_id,
								int original_factor_index,
								int new_node_id,
								int new_factor_index) = 0;
	virtual void replace_obs_node(Scope* scope,
								  int original_node_id,
								  int new_node_id) = 0;
	virtual void replace_scope(Scope* original_scope,
							   Scope* new_scope,
							   int new_scope_node_id) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	bool is_active;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */