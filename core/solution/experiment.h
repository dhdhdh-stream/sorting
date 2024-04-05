#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <set>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class BranchNode;
class ExitNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int EXPERIMENT_STATE_TRAIN_EXISTING = 0;
/**
 * - select first that is significant improvement
 *   - don't select "best" as might not have been learned for actual best
 *     - so may select lottery instead of actual best
 * 
 */
const int EXPERIMENT_STATE_EXPLORE_CREATE = 1;
const int EXPERIMENT_STATE_EXPLORE_MEASURE = 2;
const int EXPERIMENT_STATE_TRAIN_NEW = 3;
/**
 * - don't worry about retraining with new decision making
 *   - more likely to cause thrasing than to actually be helpful
 *   - simply hope that things work out, and if not, will be caught by MEASURE
 */
const int EXPERIMENT_STATE_MEASURE = 4;
const int EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
const int EXPERIMENT_STATE_VERIFY_1ST = 6;
const int EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
const int EXPERIMENT_STATE_VERIFY_2ND = 8;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_STATE_CAPTURE_VERIFY = 9;
#endif /* MDEBUG */
const int EXPERIMENT_STATE_ROOT_VERIFY = 10;
const int EXPERIMENT_STATE_EXPERIMENT = 11;
const int EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING = 12;
const int EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST = 13;
const int EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING = 14;
const int EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND = 15;

const int MAX_EXPLORE_TRIES = 4;

const double EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT = 0.05;
const double PASS_THROUGH_BRANCH_WEIGHT = 0.9;

const int MAX_EXPERIMENT_NUM_EXPERIMENTS = 20;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class ExperimentHistory;
class Experiment {
public:
	bool skip_explore;

	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;
	int throw_id;

	Experiment* parent_experiment;
	Experiment* root_experiment;

	double average_remaining_experiments_from_start;
	double average_instances_per_run;

	int state;
	int state_iter;
	int explore_iter;
	int experiment_iter;

	int result;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	int input_max_depth;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<ScopeNode*> scopes;
	std::vector<std::set<int>> catch_throw_ids;
	int exit_depth;
	AbstractNode* exit_next_node;
	int exit_throw_id;

	BranchNode* branch_node;
	ExitNode* exit_node;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_average_misguess;
	double new_misguess_standard_deviation;

	double combined_score;
	int original_count;
	int branch_count;
	double branch_weight;

	/**
	 * - don't reuse previous to not affect decision making
	 */
	double verify_existing_average_score;
	double verify_existing_score_standard_deviation;

	bool new_is_better;

	bool is_pass_through;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	std::vector<Experiment*> child_experiments;

	/**
	 * - for root
	 */
	std::vector<Experiment*> verify_experiments;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	Experiment(std::vector<Scope*> scope_context,
			   std::vector<AbstractNode*> node_context,
			   bool is_branch,
			   int throw_id,
			   Experiment* parent_experiment,
			   bool skip_explore);
	~Experiment();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void train_existing_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 ExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 RunHelper& run_helper);

	void explore_create_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 ExperimentHistory* history);
	void explore_create_backprop(double target_val,
								 RunHelper& run_helper);

	void explore_measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  std::vector<ContextLayer>& context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  RunHelper& run_helper,
								  ExperimentHistory* history);
	void explore_measure_backprop(double target_val,
								  RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper,
							ExperimentHistory* history);
	void train_new_target_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   AbstractNode*& exit_node,
								   RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ExperimentHistory* history);
	void experiment_backprop(double target_val,
							 RunHelper& run_helper);

	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	void experiment_verify_activate(AbstractNode*& curr_node,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void experiment_verify_backprop(double target_val,
									RunHelper& run_helper);

	void finalize();
	void new_branch();
	void new_pass_through();
};

class ExperimentHistory {
public:
	Experiment* experiment;

	int instance_count;

	bool has_target;
	double existing_predicted_score;

	std::vector<Experiment*> experiments_seen_order;

	ScopeHistory* scope_history;
	std::vector<int> experiment_index;

	ExperimentHistory(Experiment* experiment);
	~ExperimentHistory();
};

#endif /* EXPERIMENT_H */