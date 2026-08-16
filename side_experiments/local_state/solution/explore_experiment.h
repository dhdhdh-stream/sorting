/**
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
const int EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE = 3;
const int EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE = 4;

class ExploreExperimentHistory;
class ExploreExperiment : public AbstractExperiment {
public:
	bool use_signal;

	int state;
	int state_iter;

	std::vector<std::vector<int>> dependencies;

	double existing_val_average;

	ScoreNetwork* existing_network;

	int num_instances_until_target;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_indexes;

	std::vector<std::vector<bool>> existing_dependencies_is_hit_histories;
	std::vector<std::vector<Eigen::VectorXf>> existing_dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> existing_dependencies_obs_histories;
	std::vector<Eigen::VectorXf> existing_state_histories;
	std::vector<double> existing_signal_histories;
	std::vector<double> existing_target_val_histories;

	std::vector<std::vector<bool>> new_dependencies_is_hit_histories;
	std::vector<std::vector<Eigen::VectorXf>> new_dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> new_dependencies_obs_histories;
	std::vector<Eigen::VectorXf> new_state_histories;
	std::vector<double> new_signal_histories;
	std::vector<double> new_target_val_histories;

	ScoreNetwork* new_network;
	std::vector<InitNetwork*> init_networks;

	double sum_vals;

	ExploreExperiment(int diversity_index,
					  Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  std::vector<std::vector<int>>& dependencies,
					  bool use_signal,
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
				  ExploreExperimentHistory* history,
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

	void reuse_measure_check_activate(std::vector<double>& obs,
									  ExploreExperimentHistory* history,
									  SolutionWrapper* wrapper);
	void reuse_measure_step(std::vector<double>& obs,
							int& action,
							bool& is_next,
							SolutionWrapper* wrapper);
	void reuse_measure_callback(std::vector<double>& obs,
								SolutionWrapper* wrapper);
	void reuse_measure_exit_step(std::vector<double>& obs,
								 SolutionWrapper* wrapper);
	void reuse_measure_backprop(double target_val,
								ExploreExperimentHistory* history,
								SolutionWrapper* wrapper);

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

	void add(bool is_new_state,
			 SolutionWrapper* wrapper);

	bool further_than(ExploreExperiment* other);
};

class ExploreExperimentHistory : public AbstractExperimentHistory{
public:
	std::vector<double> existing_predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_indexes;

	std::vector<std::vector<bool>> dependencies_is_hit_histories;
	std::vector<std::vector<Eigen::VectorXf>> dependencies_state_histories;
	std::vector<std::vector<std::vector<double>>> dependencies_obs_histories;
	std::vector<Eigen::VectorXf> state_histories;
	std::vector<double> signal_histories;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */