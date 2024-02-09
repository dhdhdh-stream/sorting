/**
 * TODO:
 * - look for RetrainBranchExperiments using new path
 */

#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 5;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW = 6;
/**
 * - a successful, safe improvement only needs to be:
 *   - no score impact passthrough (but potentially information)
 *   - successful branch
 */
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 7;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING = 8;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW = 9;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING = 10;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW = 11;
/**
 * - share state_iter during experiment verify
 */

class PassThroughExperimentOverallHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;
	int curr_exit_depth;
	AbstractNode* curr_exit_node;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_node;

	bool new_is_better;

	std::map<AbstractNode*, int> node_to_step_index;

	std::vector<double> o_target_val_histories;

	int branch_experiment_step_index;
	BranchExperiment* branch_experiment;




};

class PassThroughExperimentInstanceHistory : public AbstractExperimentHistory {
public:
	std::vector<void*> pre_step_histories;

	BranchExperimentInstanceHistory* branch_experiment_history;

	std::vector<void*> post_step_histories;

	PassThroughExperimentInstanceHistory(PassThroughExperiment* experiment);
	PassThroughExperimentInstanceHistory(PassThroughExperimentInstanceHistory* original);
	~PassThroughExperimentInstanceHistory();
};

class PassThroughExperimentOverallHistory : public AbstractExperimentHistory {
public:
	BranchExperimentOverallHistory* branch_experiment_history;

	PassThroughExperimentOverallHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */