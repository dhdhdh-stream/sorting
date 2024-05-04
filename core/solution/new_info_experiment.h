#ifndef NEW_INFO_EXPERIMENT_H
#define NEW_INFO_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

const int NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO = 1;
const int NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE = 2;
const int NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW = 3;
const int NEW_INFO_EXPERIMENT_STATE_MEASURE = 4;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 5;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY_1ST = 6;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 7;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY_2ND = 8;
const int NEW_INFO_EXPERIMENT_STATE_ROOT_VERIFY = 9;
const int NEW_INFO_EXPERIMENT_STATE_EXPERIMENT = 10;

class NewInfoExperimentHistory;
class NewInfoExperiment : public AbstractExperiment {
public:
	int num_instances_until_target;

	int state;
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	double info_score;
	Scope* new_info_subscope;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	AbstractNode* best_exit_next_node;

	ActionNode* ending_node;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

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

	double verify_existing_average_score;

	bool is_pass_through;

	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	NewInfoExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractExperiment* parent_experiment);
	~NewInfoExperiment();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(NewInfoExperimentHistory* history);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_info_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   RunHelper& run_helper,
							   NewInfoExperimentHistory* history);
	void explore_info_backprop(double target_val,
							   RunHelper& run_helper);

	bool explore_sequence_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   NewInfoExperimentHistory* history);
	void explore_sequence_backprop(double target_val,
								   RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							RunHelper& run_helper,
							NewInfoExperimentHistory* history);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	bool measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	bool verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	bool root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  RunHelper& run_helper);

	bool experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 NewInfoExperimentHistory* history);
	void experiment_backprop(double target_val,
							 RunHelper& run_helper);

	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	bool experiment_verify_activate(AbstractNode*& curr_node,
									Problem* problem,
									RunHelper& run_helper);
	void experiment_verify_backprop(double target_val,
									RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class NewInfoExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double existing_predicted_score;

	NewInfoExperimentHistory(NewInfoExperiment* experiment);
	~NewInfoExperimentHistory();
};

#endif /* NEW_INFO_EXPERIMENT_H */