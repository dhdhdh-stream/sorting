#ifndef SEED_EXPERIMENT_H
#define SEED_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class ExitNode;
class Network;
class Scope;
class ScopeHistory;
class ScopeNode;
class SeedExperimentFilter;
class SeedExperimentGather;

const int SEED_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int SEED_EXPERIMENT_STATE_EXPLORE = 1;
/**
 * - simply search on path
 *   - search ascending and prioritize closer to current
 *     - continue until end or fail
 * 
 * - 1st time at FIND_FILTER, simply use explore node
 */
const int SEED_EXPERIMENT_STATE_FIND_FILTER = 2;
/**
 * - 1st time at FIND_GATHER, instead go directly to TRAIN_FILTER
 */
const int SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER = 3;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER = 4;
/**
 * - a valid gather needs to:
 *   - at curr_filter, taking seed path still hits high scores
 *   - all previous gathers/filters still valid
 *     - i.e., if filtered before curr_filter, still maintains original score
 *       - relative to new gather starting point
 */
const int SEED_EXPERIMENT_STATE_FIND_GATHER = 5;
const int SEED_EXPERIMENT_STATE_FIND_GATHER_SEED = 6;
const int SEED_EXPERIMENT_STATE_FIND_GATHER_FILTER = 7;
const int SEED_EXPERIMENT_STATE_FIND_GATHER_EXISTING = 8;
const int SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_SEED = 9;
const int SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_FILTER = 10;
const int SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER_EXISTING = 11;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER_SEED = 12;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER_FILTER = 13;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER_EXISTING = 14;
/**
 * - train on whether better than standard deviation away
 *   - allow overshoot
 * 
 * - remove spots that are >90% not going to be better
 *   - succeed if seed path higher ratio increases significantly
 * 
 * - for non-target, previous gathers/filters still activate as usual
 *   - but curr_filter will never take seed path
 */
const int SEED_EXPERIMENT_STATE_TRAIN_FILTER = 15;
const int SEED_EXPERIMENT_STATE_MEASURE_FILTER = 16;
const int SEED_EXPERIMENT_STATE_MEASURE = 17;
const int SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 18;
const int SEED_EXPERIMENT_STATE_VERIFY_1ST = 19;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 20;
const int SEED_EXPERIMENT_STATE_VERIFY_2ND = 21;
#if defined(MDEBUG) && MDEBUG
const int SEED_EXPERIMENT_STATE_CAPTURE_VERIFY = 22;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int FIND_FILTER_ITER_LIMIT = 20;
const int FIND_GATHER_ITER_LIMIT = 20;
const int FIND_GATHER_NUM_SAMPLES_PER_ITER = 2;
const int TRAIN_FILTER_ITER_LIMIT = 2;
const int TRAIN_GATHER_ITER_LIMIT = 2;
#else
const int FIND_FILTER_ITER_LIMIT = 200;
const int FIND_GATHER_ITER_LIMIT = 200;
const int FIND_GATHER_NUM_SAMPLES_PER_ITER = 20;
const int TRAIN_FILTER_ITER_LIMIT = 20;
const int TRAIN_GATHER_ITER_LIMIT = 20;
#endif /* MDEBUG */

/**
 * - filter only when extremely confident in case initially biased against seed
 *   - but take seed path only if extremely confident for final filter
 */
const double FILTER_CONFIDENCE_THRESHOLD = 0.2;
const double FILTER_FINAL_CONFIDENCE_THRESHOLD = 0.8;

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
	 * 
	 * - update based on latest filter
	 */
	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	int train_filter_iter;
	int train_gather_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> existing_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> existing_input_node_contexts;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

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

	int curr_filter_exceeded_limit_count;
	double curr_filter_score;
	int curr_filter_step_index;
	SeedExperimentFilter* curr_filter;
	bool curr_filter_is_success;

	/**
	 * - add to front of experiments, so can override previous gathers
	 *   - if override filter, then higher_ratio will likely decrease and fail
	 */
	double curr_gather_existing_average_score;
	double curr_gather_existing_score_standard_deviation;
	int curr_gather_miss_count;
	double curr_gather_seed_score;
	int curr_gather_is_higher;
	double curr_gather_non_seed_score;
	SeedExperimentGather* curr_gather;

	std::vector<SeedExperimentFilter*> filters;
	/**
	 * - 0 is original explore node
	 * - 1 is start of sequence
	 */
	int filter_step_index;
	std::vector<SeedExperimentGather*> gathers;

	double combined_score;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;
	std::vector<bool> i_is_seed_histories;
	std::vector<bool> i_is_higher_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	SeedExperiment(std::vector<Scope*> scope_context,
				   std::vector<AbstractNode*> node_context,
				   bool is_branch);
	~SeedExperiment();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  AbstractExperimentHistory* history);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper,
								 SeedExperimentOverallHistory* history);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void explore_target_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void explore_backprop(double target_val,
						  SeedExperimentOverallHistory* history);

	void create_filter();

	void find_filter_backprop(double target_val,
							  RunHelper& run_helper,
							  SeedExperimentOverallHistory* history);

	void verify_filter_backprop(double target_val,
								SeedExperimentOverallHistory* history);

	void find_gather_backprop(SeedExperimentOverallHistory* history);

	void find_gather_seed_backprop(double target_val,
								   SeedExperimentOverallHistory* history);

	void find_gather_filter_backprop(double target_val,
									 SeedExperimentOverallHistory* history);

	void find_gather_existing_backprop(double target_val,
									   RunHelper& run_helper,
									   SeedExperimentOverallHistory* history);

	void verify_gather_seed_backprop(double target_val,
									 SeedExperimentOverallHistory* history);

	void verify_gather_filter_backprop(double target_val,
									   SeedExperimentOverallHistory* history);

	void verify_gather_existing_backprop(double target_val,
										 RunHelper& run_helper,
										 SeedExperimentOverallHistory* history);

	void train_filter_backprop(double target_val,
							   SeedExperimentOverallHistory* history);

	void measure_filter_backprop(double target_val,
								 SeedExperimentOverallHistory* history);

	void measure_backprop(double target_val);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper,
								  SeedExperimentOverallHistory* history);

	void verify_backprop(double target_val);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize();
	void finalize_success();
	void clean_fail();
};

class SeedExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	SeedExperimentOverallHistory(SeedExperiment* experiment);
};

#endif /* SEED_EXPERIMENT_H */