/**
 * - don't bother with incidental pass_through
 *   - impact low
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class BranchNode;
class ExploreInstance;
class Network;
class Scope;
class SolutionWrapper;

const int EXPERIMENT_STATE_TRAIN_NEW = 0;
/**
 * - don't bother with trying to refine
 *   - impact low
 *     - not worth the samples
 */
const int EXPERIMENT_STATE_MEASURE = 1;
#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_STATE_CAPTURE_VERIFY = 2;
#endif /* MDEBUG */

class ExperimentHistory;
class ExperimentState;
class Experiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	Network* new_val_network;

	std::vector<AbstractNode*> new_nodes;

	int total_count;
	double total_sum_scores;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	double sum_scores;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	Experiment(ExploreInstance* explore_instance);
	~Experiment();

	void result_check_activate(AbstractNode* experiment_node,
							   bool is_branch,
							   SolutionWrapper* wrapper);
	void result_backprop(double target_val,
						 SolutionWrapper* wrapper);

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

	void train_new_result_check_activate(SolutionWrapper* wrapper);
	void train_new_result_backprop(SolutionWrapper* wrapper);

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 ExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_result_backprop(double target_val,
								 SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_exit_step(SolutionWrapper* wrapper,
						   ExperimentState* experiment_state);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper);
	void capture_verify_exit_step(SolutionWrapper* wrapper,
								  ExperimentState* experiment_state);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void clean();
	void add(SolutionWrapper* wrapper);
	double calc_new_score();
};

class ExperimentHistory : public AbstractExperimentHistory {
public:
	int num_instances;

	int explore_index;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */