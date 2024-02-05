#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 1;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE = 2;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 4;
/**
 * - if has parent_pass_through_experiment, skip and verify in parent
 */
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
const int BRANCH_EXPERIMENT_STATE_VERIFY_1ST = 6;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
const int BRANCH_EXPERIMENT_STATE_VERIFY_2ND = 8;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 9;
#endif /* MDEBUG */

const int BRANCH_EXPERIMENT_STATE_FAIL = 10;
const int BRANCH_EXPERIMENT_STATE_SUCCESS = 11;

const int LINEAR_NUM_OBS = 50;
const int NETWORK_INCREMENT_NUM_NEW = 10;

class BranchExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	PassThroughExperiment* parent_pass_through_experiment;

	double average_instances_per_run;
	/**
	 * - when triggering an experiment, it becomes live everywhere
	 *   - for one selected instance, always trigger branch and experiment
	 *     - for everywhere else, trigger accordingly
	 * 
	 * - set probabilities after average_instances_per_run to 50%
	 * 
	 * - average_instances_per_run changes between existing and new, as well as with new states
	 *   - so constantly best effort update
	 */

	int state;
	int state_iter;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_variance;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;
	int curr_exit_depth;
	AbstractNode* curr_exit_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_node;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_average_misguess;
	double new_misguess_variance;

	double combined_score;
	/**
	 * - if original count is 0, then is_pass_through
	 */
	int original_count;
	int branch_count;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */


};

#endif /* BRANCH_EXPERIMENT_H */