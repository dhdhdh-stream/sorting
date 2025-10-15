#ifndef REFINE_EXPERIMENT_H
#define REFINE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "input.h"

class AbstractNode;
class Network;
class Scope;
class SolutionWrapper;

const int STATUS_TYPE_DONE = 0;
const int STATUS_TYPE_NEED_VS = 1;

class RefineExperimentHistory;
class RefineExperimentState;
class RefineExperiment : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;
	AbstractNode* exit_next_node;

	int num_refines;
	int state_iter;

	double existing_constant;
	std::vector<Input> existing_inputs;
	std::vector<double> existing_input_averages;
	std::vector<double> existing_input_standard_deviations;
	std::vector<double> existing_weights;
	std::vector<Input> existing_network_inputs;
	Network* existing_network;

	double select_percentage;

	double new_constant;
	std::vector<Input> new_inputs;
	std::vector<double> new_input_averages;
	std::vector<double> new_input_standard_deviations;
	std::vector<double> new_weights;
	std::vector<Input> new_network_inputs;
	Network* new_network;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	std::vector<std::vector<double>> existing_factor_vals;
	std::vector<std::vector<double>> existing_network_vals;
	std::vector<std::vector<bool>> existing_network_is_on;
	std::vector<double> existing_target_vals;

	std::vector<std::vector<double>> new_factor_vals;
	std::vector<std::vector<double>> new_network_vals;
	std::vector<std::vector<bool>> new_network_is_on;
	std::vector<int> new_target_val_status;
	std::vector<double> new_target_vals;
	/**
	 * - if is branch, save true, and contrast later against predicted existing
	 * - it not branch, save predicted new
	 *   - to make train distribution still representative of true distribution
	 */
	std::vector<std::vector<double>> new_existing_factor_vals;
	std::vector<std::vector<double>> new_existing_network_vals;
	std::vector<std::vector<bool>> new_existing_network_is_on;

	RefineExperiment();
	~RefineExperiment();

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
				  RefineExperimentHistory* history,
				  SolutionWrapper* wrapper);

	void calc_vs();
	bool refine();

	void clean_inputs(Scope* scope,
					  int node_id);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
};

class RefineExperimentHistory {
public:
	bool is_on;

	std::vector<double> sum_signal_vals;
	std::vector<int> sum_counts;
	std::vector<std::vector<double>> new_factor_vals;
	std::vector<std::vector<double>> new_network_vals;
	std::vector<std::vector<bool>> new_network_is_on;
	std::vector<std::vector<double>> existing_factor_vals;
	std::vector<std::vector<double>> existing_network_vals;
	std::vector<std::vector<bool>> existing_network_is_on;

	RefineExperimentHistory();
};

class RefineExperimentState : public AbstractExperimentState {
public:
	int step_index;

	RefineExperimentState(RefineExperiment* experiment);
};

#endif /* REFINE_EXPERIMENT_H */