#ifndef PREDICT_EXPERIMENT_H
#define PREDICT_EXPERIMENT_H

#include <vector>

#include <Eigen/Dense>

#include "abstract_experiment.h"

class ScoreNetwork;

const int PREDICT_EXPERIMENT_STATE_GATHER_EXISTING = 0;
const int PREDICT_EXPERIMENT_STATE_MEASURE = 1;
/**
 * - need measure phase
 *   - otherwise, can be too damaging to existing if predict bad
 */

class PredictExperimentHistory;
class PredictExperiment : public AbstractExperiment {
public:
	bool use_signal;

	std::vector<Eigen::VectorXf> existing_state_histories;
	std::vector<double> existing_signal_histories;
	std::vector<double> existing_target_val_histories;

	double existing_val_average;

	ScoreNetwork* existing_network;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<int> best_indexes;

	ScoreNetwork* new_network;

	double sum_vals;

	PredictExperiment(int diversity_index,
					  Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  bool use_signal);
	~PredictExperiment();

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

	void gather_existing_check_activate(std::vector<double>& obs,
										PredictExperimentHistory* history,
										SolutionWrapper* wrapper);
	void gather_existing_backprop(double target_val,
								  PredictExperimentHistory* history,
								  SolutionWrapper* wrapper);

	void train_existing_helper();
	void explore_helper();
	void train_new_helper(SolutionWrapper* wrapper);

	void measure_check_activate(std::vector<double>& obs,
								PredictExperimentHistory* history,
								SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_callback(std::vector<double>& obs,
						  SolutionWrapper* wrapper);
	void measure_exit_step(std::vector<double>& obs,
						   SolutionWrapper* wrapper);
	void measure_backprop(double target_val,
						  PredictExperimentHistory* history,
						  SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);
};

class PredictExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<Eigen::VectorXf> state_histories;

	PredictExperimentHistory(PredictExperiment* experiment);
};

class PredictExperimentState : public AbstractExperimentState {
public:
	int step_index;

	PredictExperimentState(PredictExperiment* experiment);
};

#endif /* PREDICT_EXPERIMENT_H */