/**
 * - don't bother with incidental pass_through
 *   - impact low
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPERIMENT_STATE_EXPLORE = 1;
const int EXPERIMENT_STATE_TRAIN_NEW = 2;
/**
 * - don't bother with trying to refine
 *   - impact low
 *     - not worth the samples
 */
const int EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

class Experiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_score;

	Network* existing_network;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	double average_hits_per_run;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;
	std::vector<AbstractNode*> curr_new_nodes;

	double best_surprise;
	// temp
	double best_signal;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;
	std::vector<AbstractNode*> best_new_nodes;

	Network* new_network;

	double sum_scores;

	double total_sum_scores;
	int total_count;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	// temp
	double sum_existing_true;
	double sum_existing_signal;

	// temp
	double sum_new_true;
	double sum_new_signal;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	Experiment(Scope* scope_context,
			   AbstractNode* node_context,
			   bool is_branch,
			   SolutionWrapper* wrapper);
	~Experiment();

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

	void train_existing_check_activate(SolutionWrapper* wrapper);
	void train_existing_step(std::vector<double>& obs,
							 SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 SolutionWrapper* wrapper);

	void explore_check_activate(SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_exit_step(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_exit_step(SolutionWrapper* wrapper);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper);
	void capture_verify_exit_step(SolutionWrapper* wrapper);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void clean();
	void add(SolutionWrapper* wrapper);
	double calc_new_score();
};

class ExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted_scores;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */