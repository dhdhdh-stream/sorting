#ifndef SEED_EXPERIMENT_H
#define SEED_EXPERIMENT_H

const int SEED_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int SEED_EXPERIMENT_STATE_EXPLORE = 1;
/**
 * - after explore, simply start with MEASURE
 */
/**
 * - 1st time at FIND_FILTER, simply use explore node
 */
const int SEED_EXPERIMENT_STATE_FIND_FILTER = 2;
/**
 * - 1st time at FIND_GATHER, instead go directly to TRAIN_FILTER
 */
const int SEED_EXPERIMENT_STATE_VERIFY_FILTER = 3;
const int SEED_EXPERIMENT_STATE_FIND_GATHER = 4;
const int SEED_EXPERIMENT_STATE_VERIFY_GATHER = 5;
/**
 * - train on whether better than standard deviation away
 *   - allow overshoot
 * 
 * - remove spots that are >80% not going to be better
 *   - succeed if over 10% removed
 */
const int SEED_EXPERIMENT_STATE_TRAIN_FILTER = 6;
const int SEED_EXPERIMENT_STATE_MEASURE = 7;
const int SEED_EXPERIMENT_STATE_VERIFY_EXISTING = 8;
const int SEED_EXPERIMENT_STATE_VERIFY = 9;
/**
 * - share state_iter, sub_state_iter
 *   - state_iter for filter
 *   - sub_state_iter for gather
 */
#if defined(MDEBUG) && MDEBUG
const int SEED_EXPERIMENT_STATE_CAPTURE_VERIFY = 10;
#endif /* MDEBUG */

const int FILTER_ITER_LIMIT = 20;
const int GATHER_ITER_LIMIT = 20;

const double FILTER_CONFIDENCE_THRESHOLD = 0.2;

class SeedExperimentOverallHistory;
class SeedExperiment : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;

	/**
	 * - simply assume multiple instances in one run are independent
	 *   - if not, will simply be caught on measure and fail
	 * 
	 * - but during experiment, only trigger once per run
	 *   - as assume sequence will only be correct rarely
	 */
	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::vector<Scope*>> existing_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> existing_input_node_contexts;

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

	std::vector<Scope*> curr_filter_scope_context;
	std::vector<AbstractNode*> curr_filter_node_context;
	bool curr_filter_is_branch;
	std::vector<int> curr_filter_step_types;
	std::vector<ActionNode*> curr_filter_actions;
	std::vector<ScopeNode*> curr_filter_existing_scopes;
	std::vector<ScopeNode*> curr_filter_potential_scopes;
	int curr_filter_exit_depth;
	AbstractNode* curr_filter_next_node;

	std::vector<Scope*> best_filter_scope_context;
	std::vector<AbstractNode*> best_filter_node_context;
	bool best_filter_is_branch;
	std::vector<int> best_filter_step_types;
	std::vector<ActionNode*> best_filter_actions;
	std::vector<ScopeNode*> best_filter_existing_scopes;
	std::vector<ScopeNode*> best_filter_potential_scopes;
	int best_filter_exit_depth;
	AbstractNode* best_filter_next_node;

	std::vector<std::vector<Scope*>> curr_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> curr_input_node_contexts;

	double curr_average_confidence;
	/**
	 * - clean immediately after linear regression
	 */
	std::vector<double> curr_linear_weights;
	std::vector<std::vector<int>> curr_network_input_indexes;
	Network* curr_network;
	double curr_average_misguess;
	double curr_misguess_variance;

	std::vector<Scope*> curr_gather_scope_context;
	std::vector<AbstractNode*> curr_gather_node_context;
	bool curr_gather_is_branch;
	std::vector<int> curr_gather_step_types;
	std::vector<ActionNode*> curr_gather_actions;
	std::vector<ScopeNode*> curr_gather_existing_scopes;
	std::vector<ScopeNode*> curr_gather_potential_scopes;
	int curr_gather_exit_depth;
	AbstractNode* curr_gather_exit_node;

	std::vector<Scope*> best_gather_scope_context;
	std::vector<AbstractNode*> best_gather_node_context;
	bool best_gather_is_branch;
	std::vector<int> best_gather_step_types;
	std::vector<ActionNode*> best_gather_actions;
	std::vector<ScopeNode*> best_gather_existing_scopes;
	std::vector<ScopeNode*> best_gather_potential_scopes;
	int best_gather_exit_depth;
	AbstractNode* best_gather_exit_node;

	SeedExperimentGather* curr_gather;
	std::vector<SeedExperimentGather*> curr_gathers;

	std::vector<SeedExperimentFilter*> filters;
	std::vector<SeedExperimentGather*> gathers;

	double combined_score;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

};

class SeedExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	SeedExperimentOverallHistory(SeedExperiment* experiment);
};

#endif /* SEED_EXPERIMENT_H */