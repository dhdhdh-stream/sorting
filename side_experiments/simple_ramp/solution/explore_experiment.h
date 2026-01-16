#ifndef EXPLORE_EXPERIMENT_H
#define EXPLORE_EXPERIMENT_H

#include <list>
#include <map>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class ObsNode;
class Scope;
class ScopeHistory;
class SolutionWrapper;

const int EXPLORE_EXPERIMENT_STATE_EXPLORE = 0;
const int EXPLORE_EXPERIMENT_STATE_TRAIN_NEW = 1;

class ExploreExperimentHistory;
class ExploreExperimentState;
class ExploreExperiment : public AbstractExperiment {
public:
	ObsNode* node_context;
	AbstractNode* exit_next_node;

	std::list<int> last_num_following_explores;
	int sum_num_following_explores;
	std::list<int> last_num_instances;
	int sum_num_instances;

	int state;
	int state_iter;

	Network* existing_network;

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

	Network* new_network;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	ExploreExperiment(ObsNode* node_context,
					  AbstractNode* exit_next_node,
					  Network*& existing_network);
	~ExploreExperiment();

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
				  ExploreExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void explore_check_activate(std::vector<double>& obs,
								SolutionWrapper* wrapper,
								ExploreExperimentHistory* history);
	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  bool& fetch_action,
					  SolutionWrapper* wrapper,
					  ExploreExperimentState* experiment_state);
	void explore_set_action(int action,
							ExploreExperimentState* experiment_state);
	void explore_exit_step(SolutionWrapper* wrapper,
						   ExploreExperimentState* experiment_state);
	void explore_backprop(double target_val,
						  ExploreExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void train_new_check_activate(std::vector<double>& obs,
								  SolutionWrapper* wrapper,
								  ExploreExperimentHistory* history);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper,
						ExploreExperimentState* experiment_state);
	void train_new_exit_step(SolutionWrapper* wrapper,
							 ExploreExperimentState* experiment_state);
	void train_new_backprop(double target_val,
							ExploreExperimentHistory* history,
							SolutionWrapper* wrapper);
};

class ExploreExperimentHistory {
public:
	bool is_on;

	int num_instances;

	std::vector<double> existing_predicted_scores;

	ExploreExperimentHistory(ExploreExperiment* experiment,
							 SolutionWrapper* wrapper);
};

class ExploreExperimentState : public AbstractExperimentState {
public:
	int step_index;

	ExploreExperimentState(ExploreExperiment* experiment);
};

bool train_existing_helper(ObsNode* node_context,
						   SolutionWrapper* wrapper,
						   Network*& network);

#endif /* EXPLORE_EXPERIMENT_H */