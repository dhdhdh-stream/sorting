#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <set>
#include <vector>

#include <Eigen/Dense>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
/**
 * - simply always retrain existing_network
 *   - solution is always changing, so keeping an overall existing_network updated significant cost
 *   - also much easier to make mistakes
 *     - in case branches not properly taken into account
 */
const int EXPLORE_EXPERIMENT_STATE_PREDICT_MEASURE = 1;
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 2;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 3;

class ExploreExperimentHistory;
class ExploreExperiment : public AbstractExperiment {
public:
	std::vector<std::vector<std::vector<double>>> existing_obs_histories;
	std::vector<std::vector<std::vector<AbstractNode*>>> existing_node_context_histories;
	std::vector<std::vector<Eigen::VectorXf>> existing_state_histories;
	/**
	 * - invalidated once predict updated
	 *   - so round of predict after existing, before explore
	 */
	std::vector<double> existing_target_val_histories;
	int existing_index;

	Network* existing_network;

	double sum_improvement;

	int num_instances_until_target;

	std::vector<double> surprises;

	std::vector<int> best_step_types;
	std::vector<int> best_indexes;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	Network* new_network;

	ExploreExperiment(int diversity_index,
					  Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper);
	~ExploreExperiment();

	void experiment_check_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_step_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);
	void backprop(double target_val,
				  AbstractExperimentHistory* history,
				  SolutionWrapper* wrapper,
				  bool& is_add);

	void train_existing_check_activate(std::vector<double>& obs,
									   ExploreExperimentHistory* history,
									   SolutionWrapper* wrapper);
	void train_existing_backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper);

	void train_existing_helper(SolutionWrapper* wrapper);
	bool predict_cycle(SolutionWrapper* wrapper);

	void predict_measure_check_activate(std::vector<double>& obs,
										ExploreExperimentHistory* history,
										SolutionWrapper* wrapper);
	void predict_measure_step(std::vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper);
	void predict_measure_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);
	void predict_measure_exit_step(std::vector<double>& obs,
								   SolutionWrapper* wrapper);
	void predict_measure_backprop(double target_val,
								  ExploreExperimentHistory* history,
								  SolutionWrapper* wrapper,
								  bool& is_add);

	void explore_check_activate(std::vector<double>& obs,
								ExploreExperimentHistory* history,
								SolutionWrapper* wrapper);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
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
							SolutionWrapper* wrapper,
							bool& is_add);

	bool train_new_helper(int l_index);

	void add(bool is_predict,
			 SolutionWrapper* wrapper);
};

class ExploreExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<std::vector<double>> obs_histories;
	std::vector<std::vector<AbstractNode*>> node_context_histories;
	std::vector<Eigen::VectorXf> state_histories;

	bool has_explore;

	bool has_predict;

	double existing_predicted;
	double predicted;

	std::vector<int> curr_step_types;
	std::vector<int> curr_indexes;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */