#ifndef SEED_EXPERIMENT_H
#define SEED_EXPERIMENT_H

const int SEED_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int SEED_EXPERIMENT_STATE_EXPLORE = 1;
const int SEED_EXPERIMENT_STATE_MEASURE_SEED = 2;
/**
 * - 1st time at FIND_FILTER, simply use explore node
 */
const int SEED_EXPERIMENT_STATE_FIND_FILTER = 3;
/**
 * - 1st time at FIND_GATHER, instead go directly to TRAIN_FILTER
 */
const int SEED_EXPERIMENT_STATE_VERIFY_FILTER = 4;
/**
 * - a valid gather needs to:
 *   - when taking seed path, still hits high scores
 *   - at curr_filter, filtering still maintains original score
 *   - all previous gathers/filters still valid
 *     - i.e., if filtered before curr_filter, still maintains original score
 * 
 * - simply 50/50 seed path vs. non-seed path
 *   - if this->sub_state_iter%2 == 0, seed path
 */
const int SEED_EXPERIMENT_STATE_FIND_GATHER = 5;
const int SEED_EXPERIMENT_STATE_VERIFY_GATHER = 6;
/**
 * - train on whether better than standard deviation away
 *   - allow overshoot
 * 
 * - remove spots that are >80% not going to be better
 *   - succeed if over 10% not better removed and non-significant better removed
 */
const int SEED_EXPERIMENT_STATE_TRAIN_FILTER = 7;
const int SEED_EXPERIMENT_STATE_MEASURE_FILTER = 8;
const int SEED_EXPERIMENT_STATE_MEASURE = 9;
const int SEED_EXPERIMENT_STATE_VERIFY_EXISTING = 10;
const int SEED_EXPERIMENT_STATE_VERIFY = 11;
#if defined(MDEBUG) && MDEBUG
const int SEED_EXPERIMENT_STATE_CAPTURE_VERIFY = 12;
#endif /* MDEBUG */

const int EXPLORE_SEED_PATH_NUM_ITERS = 20;
const int EXPLORE_NON_SEED_PATH_NUM_ITERS = 20;

const double FILTER_CONFIDENCE_THRESHOLD = 0.2;

class SeedExperimentOverallHistory;
class SeedExperiment : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;

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

	int filter_iter;
	int gather_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

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
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_next_node;
	ExitNode* best_exit_node;

	double curr_higher_ratio;

	SeedExperimentFilter* curr_filter;

	SeedExperimentGather* curr_gather;

	std::vector<SeedExperimentFilter*> filters;
	std::vector<SeedExperimentGather*> gathers;

	double combined_score;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;
	std::vector<bool> i_is_seed_histories;
	std::vector<bool> i_is_higher_histories;

};

class SeedExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	SeedExperimentOverallHistory(SeedExperiment* experiment);
};

#endif /* SEED_EXPERIMENT_H */