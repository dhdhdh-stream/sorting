#ifndef REFINE_H
#define REFINE_H

#include "abstract_experiment.h"

#include <vector>

class Network;

class RefineHistory;
class Refine : public AbstractExperiment {
public:
	int epoch_iter;
	int run_iter;

	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	Network* existing_network;
	Network* new_network;

	std::vector<std::vector<double>> new_obs_histories;
	std::vector<double> new_target_val_histories;

	int start_iter;
	double existing_sum_scores;
	int existing_count;
	double new_sum_scores;
	int new_count;

	Refine(SolutionWrapper* wrapper);
	~Refine();

	void experiment_check_activate(std::vector<double>& obs,
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
				  RefineHistory* history,
				  SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);
};

class RefineHistory {
public:
	Refine* refine;

	bool is_active;

	std::vector<bool> is_branch;
	std::vector<std::vector<double>> obs_histories;

	RefineHistory(Refine* refine);
};

class RefineState : public AbstractExperimentState {
public:
	int step_index;

	RefineState(Refine* refine);
};

#endif /* REFINE_H */