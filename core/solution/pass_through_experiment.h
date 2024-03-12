#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <map>
#include <set>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class ExitNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_CREATE = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_MEASURE = 2;
/**
 * - share this->state_iter between EXPLORE_CREATE and EXPLORE_MEASURE
 */
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW = 5;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 6;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW = 7;
const int PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY = 8;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 9;
/**
 * - for root experiment, triggered by descendent experiment
 */
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING = 10;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW = 11;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING = 12;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW = 13;

const int MAX_PASS_THROUGH_EXPERIMENT_NUM_EXPERIMENTS = 20;

#if defined(MDEBUG) && MDEBUG
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 40;
#endif /* MDEBUG */

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_fuzzy_match;
	bool is_branch;
	int throw_id;

	PassThroughExperiment* parent_experiment;
	PassThroughExperiment* root_experiment;

	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;
	std::vector<std::set<int>> curr_catch_throw_ids;
	int curr_exit_depth;
	AbstractNode* curr_exit_next_node;
	int curr_exit_throw_id;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	std::vector<std::set<int>> best_catch_throw_ids;
	int best_exit_depth;
	AbstractNode* best_exit_next_node;
	int best_exit_throw_id;

	ExitNode* exit_node;

	bool new_is_better;

	std::vector<double> o_target_val_histories;

	std::vector<AbstractExperiment*> child_experiments;

	/**
	 * - for root
	 */
	std::vector<AbstractExperiment*> verify_experiments;

	PassThroughExperiment(std::vector<Scope*> scope_context,
						  std::vector<AbstractNode*> node_context,
						  bool is_fuzzy_match,
						  bool is_branch,
						  int throw_id,
						  PassThroughExperiment* parent_experiment);
	~PassThroughExperiment();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(PassThroughExperimentHistory* history);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper,
								   PassThroughExperimentHistory* history);

	void explore_create_activate(AbstractNode*& curr_node,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 PassThroughExperimentHistory* history);
	void explore_create_backprop(PassThroughExperimentHistory* history);

	void explore_measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  std::vector<ContextLayer>& context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  RunHelper& run_helper);
	void explore_measure_backprop(double target_val,
								  RunHelper& run_helper);

	void measure_new_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper);
	void measure_new_backprop(double target_val,
							  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_new_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 int& exit_depth,
							 AbstractNode*& exit_node,
							 RunHelper& run_helper);
	void verify_new_backprop(double target_val,
							 RunHelper& run_helper);

	void root_verify_activate(AbstractNode*& curr_node,
							  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 PassThroughExperimentHistory* history);
	void experiment_backprop(double target_val,
							 RunHelper& run_helper,
							 PassThroughExperimentHistory* history);

	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	void experiment_verify_new_activate(AbstractNode*& curr_node,
										RunHelper& run_helper);
	void experiment_verify_new_backprop(double target_val,
										RunHelper& run_helper);

	void finalize();
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	std::vector<AbstractExperiment*> experiments_seen_order;

	ScopeHistory* scope_history;
	std::vector<int> experiment_index;

	PassThroughExperimentHistory(PassThroughExperiment* experiment);
	~PassThroughExperimentHistory();
};

#endif /* PASS_THROUGH_EXPERIMENT_H */