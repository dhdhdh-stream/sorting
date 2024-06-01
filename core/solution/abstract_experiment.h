#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include "run_helper.h"
#include "context_layer.h"

class AbstractNode;
class Problem;
class Solution;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_NEW_INFO = 2;
const int EXPERIMENT_TYPE_INFO_PASS_THROUGH = 3;
const int EXPERIMENT_TYPE_NEW_ACTION = 4;

const int ROOT_EXPERIMENT_STATE_EXPERIMENT = 0;
const int ROOT_EXPERIMENT_STATE_VERIFY_EXISTING = 1;
const int ROOT_EXPERIMENT_STATE_VERIFY = 2;

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
	int score_type;

	AbstractExperiment* parent_experiment;
	AbstractExperiment* root_experiment;

	double average_remaining_experiments_from_start;
	double average_instances_per_run;

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
	virtual void back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper) = 0;
	virtual void backprop(double target_val,
						  RunHelper& run_helper) = 0;

	virtual void finalize(Solution* duplicate) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	std::vector<std::vector<double>> starting_predicted_scores;
	std::vector<std::vector<double>> ending_predicted_scores;

	int experiment_index;

	std::vector<AbstractExperiment*> experiments_seen_order;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */