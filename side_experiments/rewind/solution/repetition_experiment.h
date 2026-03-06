#ifndef REPETITION_EXPERIMENT_H
#define REPETITION_EXPERIMENT_H

#include "abstract_experiment.h"

const int REPETITION_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int REPETITION_EXPERIMENT_STATE_MEASURE_NEW = 1;

class RepetitionExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	Scope* new_scope;

	std::vector<double> existing_scores;
	std::vector<double> new_scores;

	double total_sum_scores;
	int total_count;

	double local_improvement;
	double global_improvement;
	double score_standard_deviation;

	RepetitionExperiment(AbstractNode* node_context,
						 Scope* new_scope);
	~RepetitionExperiment();

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

	void measure_existing_backprop(double target_val,
								   SolutionWrapper* wrapper);

	void measure_new_check_activate(SolutionWrapper* wrapper);
	void measure_new_step(std::vector<double>& obs,
						  int& action,
						  bool& is_next,
						  SolutionWrapper* wrapper);
	void measure_new_exit_step(SolutionWrapper* wrapper);
	void measure_new_backprop(double target_val,
							  SolutionWrapper* wrapper);

	void clean();
	void add(SolutionWrapper* wrapper);
	double calc_new_score();
};

class RepetitionExperimentHistory : public AbstractExperimentHistory {
public:
	RepetitionExperimentHistory(RepetitionExperiment* experiment);
};

class RepetitionExperimentState : public AbstractExperimentState {
public:
	RepetitionExperimentState(RepetitionExperiment* experiment);
};

#endif /* REPETITION_EXPERIMENT_H */