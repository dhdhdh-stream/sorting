#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class BranchNode;
class Network;
class Scope;
class SolutionWrapper;

const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_REFINE = 2;
/**
 * - practice for ramp
 *   - not actually better than just gathering more samples
 */
const int BRANCH_EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

class BranchExperimentHistory;
class BranchExperimentState;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_constant;
	std::vector<Input> existing_inputs;
	std::vector<double> existing_input_averages;
	std::vector<double> existing_input_standard_deviations;
	std::vector<double> existing_weights;
	std::vector<Input> existing_network_inputs;
	Network* existing_network;
	/**
	 * - simply don't bother reusing existing_network
	 */

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	double new_constant;
	std::vector<Input> new_inputs;
	std::vector<double> new_input_averages;
	std::vector<double> new_input_standard_deviations;
	std::vector<double> new_weights;
	std::vector<Input> new_network_inputs;
	Network* new_network;

	double select_percentage;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<double> target_val_histories;

	std::vector<std::vector<double>> factor_vals;
	std::vector<std::vector<double>> network_vals;
	std::vector<std::vector<bool>> network_is_on;

	int num_refines;

	double new_sum_scores;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch,
					 SolutionWrapper* wrapper);
	~BranchExperiment();

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper,
								BranchExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  BranchExperimentState* experiment_state);
	void explore_set_action(int action,
							BranchExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   BranchExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper,
								  BranchExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						BranchExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void refine_check_activate(SolutionWrapper* wrapper,
							   BranchExperimentHistory* history);
	void refine_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper,
					 BranchExperimentState* experiment_state);
	void refine_exit_step(SolutionWrapper* wrapper,
						  BranchExperimentState* experiment_state);
	void refine_backprop(double target_val,
						 SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper,
								BranchExperimentHistory* history);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper,
					  BranchExperimentState* experiment_state);
	void measure_exit_step(SolutionWrapper* wrapper,
						   BranchExperimentState* experiment_state);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper,
							 BranchExperimentState* experiment_state);
	void capture_verify_exit_step(SolutionWrapper* wrapper,
								  BranchExperimentState* experiment_state);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void train_existing_helper(std::vector<ScopeHistory*>& scope_histories,
						   std::vector<double>& target_val_histories);
	bool train_new_helper();
	bool refine_helper();

	void clean();
	void add(SolutionWrapper* wrapper);
};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_scores;

	std::vector<std::vector<double>> factor_vals;
	std::vector<std::vector<double>> network_vals;
	std::vector<std::vector<bool>> network_is_on;

	BranchExperimentHistory(BranchExperiment* experiment);
};

class BranchExperimentState : public AbstractExperimentState {
public:
	int step_index;

	BranchExperimentState(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */