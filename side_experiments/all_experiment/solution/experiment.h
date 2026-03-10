/**
 * - there will be more Experiments on long paths
 *   - so will damage long paths more
 *     - but that's preferable
 * 
 * - don't explicitly worry about passthrough
 *   - 100% vs 90% branch not significantly different in terms of fracturing
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <set>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class ObsNode;
class Scope;
class SolutionWrapper;

const int EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPERIMENT_STATE_EXPLORE = 1;
const int EXPERIMENT_STATE_TRAIN_NEW = 2;
/**
 * - retrain existing during train_new
 */
const int EXPERIMENT_STATE_RAMP = 3;
const int EXPERIMENT_STATE_MEASURE = 4;

const int MEASURE_STATUS_N_A = 0;
const int MEASURE_STATUS_SUCCESS = 1;
const int MEASURE_STATUS_FAIL = 2;

class ExperimentHistory;
class ExperimentState;
class Experiment : public AbstractExperiment {
public:
	ObsNode* node_context;
	AbstractNode* exit_next_node;

	int state;
	int state_iter;

	Network* existing_network;

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;

	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;
	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	std::vector<Network*> new_networks;

	int num_original;
	int num_branch;

	int starting_iter;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	int curr_ramp;
	/**
	 * - simply init to 0 and fast fail
	 */
	int measure_status;

	double local_improvement;
	double global_improvement;
	double score_standard_deviation;

	Experiment(ObsNode* node_context,
			   AbstractNode* exit_next_node);
	~Experiment();

	void check_activate(AbstractNode* experiment_node,
						std::vector<double>& obs,
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
				  ExperimentHistory* history,
				  SolutionWrapper* wrapper,
				  std::set<Scope*>& updated_scopes);

	void train_existing_check_activate(std::vector<double>& obs,
									   SolutionWrapper* wrapper,
									   ExperimentHistory* history);
	void train_existing_backprop(double target_val,
								 ExperimentHistory* history,
								 SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								SolutionWrapper* wrapper,
								ExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  ExperimentState* experiment_state);
	void explore_set_action(int action,
							ExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   ExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  ExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(std::vector<double>& obs,
								  SolutionWrapper* wrapper,
								  ExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						ExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 ExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							ExperimentHistory* history,
							SolutionWrapper* wrapper);

	void ramp_check_activate(std::vector<double>& obs,
							 SolutionWrapper* wrapper,
							 ExperimentHistory* history);
	void ramp_step(std::vector<double>& obs,
				   int& action,
				   bool& is_next,
				   SolutionWrapper* wrapper,
				   ExperimentState* experiment_state);
	void ramp_exit_step(SolutionWrapper* wrapper,
						ExperimentState* experiment_state);
	void ramp_backprop(double target_val,
					   ExperimentHistory* history,
					   SolutionWrapper* wrapper,
					   std::set<Scope*>& updated_scopes);

	void add(SolutionWrapper* wrapper);
};

class ExperimentHistory {
public:
	bool is_on;

	std::vector<double> existing_predicted_scores;

	bool hit_branch;

	ExperimentHistory(Experiment* experiment);
};

class ExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExperimentState(Experiment* experiment);
};

#endif /* EXPERIMENT_H */