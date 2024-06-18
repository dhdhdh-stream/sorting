#ifndef NEW_INFO_EXPERIMENT_H
#define NEW_INFO_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class InfoBranchNode;
class InfoScope;
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
const int NEW_INFO_EXPERIMENT_STATE_TRY_EXISTING_INFO = 5;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING = 6;
const int NEW_INFO_EXPERIMENT_STATE_VERIFY = 7;
#if defined(MDEBUG) && MDEBUG
const int NEW_INFO_EXPERIMENT_STATE_CAPTURE_VERIFY = 8;
#endif /* MDEBUG */
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
	InfoScope* new_info_scope;

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
	InfoBranchNode* branch_node;

	std::vector<AbstractNode*> existing_input_node_contexts;
	std::vector<int> existing_input_obs_indexes;
	Network* existing_network;

	double new_average_score;

	std::vector<AbstractNode*> new_input_node_contexts;
	std::vector<int> new_input_obs_indexes;
	Network* new_network;

	double combined_score;
	int original_count;
	int branch_count;
	double branch_weight;

	bool is_pass_through;

	bool use_existing;
	int existing_info_scope_index;
	bool existing_is_negate;

	std::vector<AbstractScopeHistory*> scope_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	NewInfoExperiment(AbstractScope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractExperiment* parent_experiment);
	~NewInfoExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void back_activate(std::vector<ContextLayer>& context,
					   RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void measure_existing_activate(std::vector<ContextLayer>& context,
								   NewInfoExperimentHistory* history);
	void measure_existing_back_activate(std::vector<ContextLayer>& context,
										RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_info_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   NewInfoExperimentHistory* history);
	void explore_info_back_activate(std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void explore_info_backprop(double target_val,
							   RunHelper& run_helper);

	bool explore_sequence_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   NewInfoExperimentHistory* history);
	void explore_sequence_back_activate(std::vector<ContextLayer>& context,
										RunHelper& run_helper);
	void explore_sequence_backprop(double target_val,
								   RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							NewInfoExperimentHistory* history);
	void train_new_back_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	bool measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  NewInfoExperimentHistory* history);
	void measure_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	bool try_existing_info_activate(AbstractNode*& curr_node,
									Problem* problem,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper,
									NewInfoExperimentHistory* history);
	void try_existing_info_back_activate(std::vector<ContextLayer>& context,
										 RunHelper& run_helper);
	void try_existing_info_backprop(double target_val,
									RunHelper& run_helper);

	void verify_existing_activate(std::vector<ContextLayer>& context,
								  NewInfoExperimentHistory* history);
	void verify_existing_back_activate(std::vector<ContextLayer>& context,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	bool verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 NewInfoExperimentHistory* history);
	void verify_back_activate(std::vector<ContextLayer>& context,
							  RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	bool capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	bool root_verify_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  RunHelper& run_helper);

	bool experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 NewInfoExperimentHistory* history);
	void experiment_back_activate(std::vector<ContextLayer>& context,
								  RunHelper& run_helper);
	void experiment_backprop(RunHelper& run_helper);

	void experiment_verify_existing_activate(std::vector<ContextLayer>& context,
											 NewInfoExperimentHistory* history);
	void experiment_verify_existing_back_activate(std::vector<ContextLayer>& context,
												  RunHelper& run_helper);
	void experiment_verify_existing_backprop(double target_val,
											 RunHelper& run_helper);

	bool experiment_verify_activate(AbstractNode*& curr_node,
									Problem* problem,
									std::vector<ContextLayer>& context,
									RunHelper& run_helper,
									NewInfoExperimentHistory* history);
	void experiment_verify_back_activate(std::vector<ContextLayer>& context,
										 RunHelper& run_helper);
	void experiment_verify_backprop(double target_val,
									RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
	void new_existing(Solution* duplicate);
};

class NewInfoExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	std::vector<double> existing_predicted_scores;

	NewInfoExperimentHistory(NewInfoExperiment* experiment);
};

#endif /* NEW_INFO_EXPERIMENT_H */