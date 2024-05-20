#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include "run_helper.h"
#include "context_layer.h"

class AbstractNode;
class EvalHistory;
class Problem;
class Solution;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_EVAL_PASS_THROUGH = 2;
const int EXPERIMENT_TYPE_NEW_INFO = 3;
const int EXPERIMENT_TYPE_INFO_PASS_THROUGH = 4;
const int EXPERIMENT_TYPE_NEW_ACTION = 5;

const int ROOT_EXPERIMENT_STATE_EXPERIMENT = 0;
const int ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 1;
const int ROOT_EXPERIMENT_STATE_VERIFY_1ST = 2;
const int ROOT_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 3;
const int ROOT_EXPERIMENT_STATE_VERIFY_2ND = 4;

const int MAX_EXPERIMENT_NUM_EXPERIMENTS = 20;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	AbstractExperiment* parent_experiment;
	AbstractExperiment* root_experiment;

	double average_remaining_experiments_from_start;

	int root_state;

	std::vector<AbstractExperiment*> child_experiments;
	int experiment_iter;

	/**
	 * - for root
	 */
	std::vector<AbstractExperiment*> verify_experiments;

	int result;

	std::vector<double> target_val_histories;

	virtual ~AbstractExperiment() {};
	virtual void decrement(AbstractNode* experiment_node) = 0;

	virtual bool activate(AbstractNode* experiment_node,
						  bool is_branch,
						  AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper) = 0;
	virtual void backprop(EvalHistory* eval_history,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper) = 0;

	virtual void finalize(Solution* duplicate) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	std::vector<AbstractExperiment*> experiments_seen_order;

	ScopeHistory* scope_history;
	int experiment_index;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */