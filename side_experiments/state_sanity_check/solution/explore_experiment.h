/**
 * - need explore even with predict
 *   - difficult to find sharp/different with predict
 *     - especially when different state needs to be captured
 * 
 * - too expensive to retrain networks
 *   - means having to activate/backprop every network
 *     - since affects error gradient
 */

#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <set>
#include <vector>

#include <Eigen/Dense>

#include "abstract_experiment.h"

class AbstractNode;
class InitNetwork;
class ScoreNetwork;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 1;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int EXPLORE_EXPERIMENT_STATE_VERIFY = 4;
#endif /* MDEBUG */

class ExploreExperimentHistory;
class ExploreExperiment : public AbstractExperiment {
public:
	std::vector<std::vector<int>> dependencies;

	double existing_val_average;

	std::vector<InitNetwork*> existing_init_networks;
	InitNetwork* existing_init_network;
	ScoreNetwork* existing_network;

	int num_instances_until_target;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_indexes;

	std::vector<std::vector<bool>> existing_dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> existing_dependencies_obs_histories;
	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;

	std::vector<std::vector<bool>> new_dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> new_dependencies_obs_histories;
	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	std::vector<InitNetwork*> new_init_networks;
	InitNetwork* new_init_network;
	ScoreNetwork* new_network;

	double sum_vals;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_run_seeds;
	std::vector<std::vector<double>> verify_original_state_vals;
	std::vector<std::vector<double>> verify_branch_state_vals;
	#endif /* MDEBUG */

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  std::vector<std::vector<int>>& dependencies,
					  SolutionWrapper* wrapper);
	~ExploreExperiment();

	void experiment_check_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_step_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);
	void backprop(double target_val,
				  AbstractExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void train_existing_check_activate(std::vector<double>& obs,
									   ExploreExperimentHistory* history,
									   SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								ExploreExperimentHistory* history,
								SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_callback(std::vector<double>& obs,
						  SolutionWrapper* wrapper);
	void explore_exit_step(std::vector<double>& obs,
						   SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(std::vector<double>& obs,
								  ExploreExperimentHistory* history,
								  SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_callback(std::vector<double>& obs,
							SolutionWrapper* wrapper);
	void train_new_exit_step(std::vector<double>& obs,
							 SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							ExploreExperimentHistory* history,
							SolutionWrapper* wrapper);

	void new_state_helper(SolutionWrapper* wrapper);

	void new_state_measure_check_activate(std::vector<double>& obs,
										  ExploreExperimentHistory* history,
										  SolutionWrapper* wrapper);
	void new_state_measure_step(std::vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper);
	void new_state_measure_callback(std::vector<double>& obs,
									SolutionWrapper* wrapper);
	void new_state_measure_exit_step(std::vector<double>& obs,
									 SolutionWrapper* wrapper);
	void new_state_measure_backprop(double target_val,
									ExploreExperimentHistory* history,
									SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void verify_check_activate(std::vector<double>& obs,
							   ExploreExperimentHistory* history,
							   SolutionWrapper* wrapper);
	void verify_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);
	void verify_callback(std::vector<double>& obs,
						 SolutionWrapper* wrapper);
	void verify_exit_step(std::vector<double>& obs,
						  SolutionWrapper* wrapper);
	void verify_backprop(double target_val,
						 ExploreExperimentHistory* history,
						 SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void add(bool is_new_state,
			 SolutionWrapper* wrapper);
};

class ExploreExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<double> existing_predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_indexes;

	std::vector<std::vector<bool>> dependencies_is_hit_histories;
	std::vector<std::vector<std::vector<double>>> dependencies_obs_histories;
	std::vector<std::vector<double>> obs_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<std::vector<double>> verify_original_state_vals;
	std::vector<std::vector<double>> verify_branch_state_vals;
	#endif /* MDEBUG */

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */