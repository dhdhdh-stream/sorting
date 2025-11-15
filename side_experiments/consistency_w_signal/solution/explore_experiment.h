#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExploreInstance;
class Network;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 1;

class ExploreExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	double existing_score;
	// // temp
	// double existing_signal;

	Network* existing_network;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	int total_count;

	double average_hits_per_run;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;
	// // temp
	// std::vector<double> signal_histories;

	double sum_scores;

	ExploreExperiment(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  SolutionWrapper* wrapper);
	~ExploreExperiment();

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

	void clean();
};

class ExploreExperimentHistory : public AbstractExperimentHistory {
public:
	ExploreInstance* explore_instance;
	double existing_predicted_score;

	ExploreExperimentHistory(ExploreExperiment* experiment);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

#endif /* EXPLORE_EXPERIMENT_H */