#ifndef SEED_EXPERIMENT_H
#define SEED_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class AbstractScope;
class AbstractScopeHistory;
class ActionNode;
class Network;
class Problem;
class ScopeNode;

const int SEED_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int SEED_EXPERIMENT_STATE_EXPLORE_SEED = 1;
const int SEED_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int SEED_EXPERIMENT_STATE_EXPLORE_BACK = 3;
const int SEED_EXPERIMENT_STATE_MEASURE = 4;
const int SEED_EXPERIMENT_STATE_VERIFY_EXISTING = 5;
const int SEED_EXPERIMENT_STATE_VERIFY = 6;
#if defined(MDEBUG) && MDEBUG
const int SEED_EXPERIMENT_STATE_CAPTURE_VERIFY = 7;
#endif /* MDEBUG */

class SeedExperimentHistory;
class SeedExperiment : public AbstractExperiment {
public:
	int num_instances_until_target;

	int state;
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double existing_average_score;

	std::vector<int> curr_seed_step_types;
	std::vector<ActionNode*> curr_seed_actions;
	std::vector<ScopeNode*> curr_seed_scopes;
	AbstractNode* curr_seed_exit_next_node;

	double best_seed_surprise;
	std::vector<int> best_seed_step_types;
	std::vector<ActionNode*> best_seed_actions;
	std::vector<ScopeNode*> best_seed_scopes;
	AbstractNode* best_seed_exit_next_node;

	std::vector<std::vector<AbstractScope*>> new_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> new_input_node_contexts;
	std::vector<int> new_input_obs_indexes;
	Network* new_network;

	bool is_pass_through;
	int branch_index;

	double curr_back_score;
	std::vector<int> curr_back_step_types;
	std::vector<ActionNode*> curr_back_actions;
	std::vector<ScopeNode*> curr_back_scopes;
	AbstractNode* curr_back_exit_next_node;

	double best_back_score;
	std::vector<int> best_back_step_types;
	std::vector<ActionNode*> best_back_actions;
	std::vector<ScopeNode*> best_back_scopes;
	AbstractNode* best_back_exit_next_node;

	double combined_score;

	std::vector<AbstractScopeHistory*> scope_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	SeedExperiment(AbstractScope* scope_context,
				   AbstractNode* node_context,
				   bool is_branch,
				   int score_type);
	~SeedExperiment();
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
								   SeedExperimentHistory* history);
	void measure_existing_back_activate(std::vector<ContextLayer>& context,
										RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void explore_seed_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   SeedExperimentHistory* history);
	void explore_seed_back_activate(std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void explore_seed_backprop(double target_val,
							   RunHelper& run_helper);

	void train_new_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							SeedExperimentHistory* history);
	void train_new_back_activate(std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_new_backprop(double target_val,
							RunHelper& run_helper);

	void explore_back_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   SeedExperimentHistory* history);
	void explore_back_back_activate(std::vector<ContextLayer>& context,
									RunHelper& run_helper);
	void explore_back_backprop(double target_val,
							   RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  SeedExperimentHistory* history);
	void measure_back_activate(std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void verify_existing_activate(std::vector<ContextLayer>& context,
								  SeedExperimentHistory* history);
	void verify_existing_back_activate(std::vector<ContextLayer>& context,
									   RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 SeedExperimentHistory* history);
	void verify_back_activate(std::vector<ContextLayer>& context,
							  RunHelper& run_helper);
	void verify_backprop(double target_val,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class SeedExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	SeedExperimentHistory(SeedExperiment* experiment);
};

#endif /* SEED_EXPERIMENT_H */