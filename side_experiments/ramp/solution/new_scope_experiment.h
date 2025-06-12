#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include "abstract_experiment.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE_1_PERCENT = 0;
const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE_5_PERCENT = 1;
const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE_10_PERCENT = 2;
const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE_25_PERCENT = 3;
const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE_50_PERCENT = 4;
const int NEW_SCOPE_EXPERIMENT_STATE_SUCCESS = 5;

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	AbstractNode* exit_next_node;

	ScopeNode* successful_scope_node;

	double existing_sum_score;
	int existing_count;
	double new_sum_score;
	int new_count;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  AbstractExperimentHistory* history,
				  std::set<Scope*>& updated_scopes);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

	void add();
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */