#ifndef CANDIDATE_EXPERIMENT_H
#define CANDIDATE_EXPERIMENT_H

#include "abstract_experiment.h"

class AbstractNode;
class Network;
class Scope;
class Tunnel;

const int CANDIDATE_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int CANDIDATE_EXPERIMENT_STATE_EXPLORE = 1;
const int CANDIDATE_EXPERIMENT_STATE_TRAIN_NEW = 2;
const int CANDIDATE_EXPERIMENT_STATE_MEASURE = 3;
#if defined(MDEBUG) && MDEBUG
const int CANDIDATE_EXPERIMENT_STATE_CAPTURE_VERIFY = 4;
#endif /* MDEBUG */

class CandidateExperiment : public AbstractExperiment {
public:
	Tunnel* candidate;

	int state;
	int state_iter;

	double existing_true;

	double candidate_starting_val_average;
	double candidate_starting_val_standard_error;

	Network* existing_true_network;
	Network* existing_signal_network;

	int sum_num_instances;

	double average_instances_per_run;
	int num_instances_until_target;

	Scope* curr_new_scope;
	std::vector<int> curr_step_types;
	std::vector<int> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;
	std::vector<AbstractNode*> curr_new_nodes;

	double best_surprise;
	Scope* best_new_scope;
	std::vector<int> best_step_types;
	std::vector<int> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;
	std::vector<AbstractNode*> best_new_nodes;

	Network* new_signal_network;

	double sum_true;
	int hit_count;
	std::vector<double> signal_vals;

	double total_sum_scores;
	int total_count;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> true_histories;
	std::vector<double> signal_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	CandidateExperiment(Scope* scope_context,
						AbstractNode* node_context,
						bool is_branch);
	~CandidateExperiment();

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

	void train_new_check_activate(SolutionWrapper* wrapper);
	void train_new_step(std::vector<double>& obs,
						int& action,
						bool& is_next,
						SolutionWrapper* wrapper);
	void train_new_exit_step(SolutionWrapper* wrapper);
	void train_new_backprop(double target_val,
							SolutionWrapper* wrapper);

	void measure_check_activate(SolutionWrapper* wrapper);
	void measure_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void measure_exit_step(SolutionWrapper* wrapper);
	void measure_backprop(double target_val,
						  SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_check_activate(SolutionWrapper* wrapper);
	void capture_verify_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper);
	void capture_verify_exit_step(SolutionWrapper* wrapper);
	void capture_verify_backprop(SolutionWrapper* wrapper);
	#endif /* MDEBUG */

	void clean();
	void add(SolutionWrapper* wrapper);
	double calc_new_score();

	void gather_tunnel_data_helper(ScopeHistory* scope_history,
								   double& sum_vals);
};

class CandidateExperimentHistory : public AbstractExperimentHistory {
public:
	std::vector<std::vector<ScopeHistory*>> stack_traces;

	std::vector<double> existing_predicted_trues;
	std::vector<double> existing_predicted_signals;

	CandidateExperimentHistory(CandidateExperiment* experiment);
};

class CandidateExperimentState : public AbstractExperimentState {
public:
	int step_index;

	CandidateExperimentState(CandidateExperiment* experiment);
};

#endif /* CANDIDATE_EXPERIMENT_H */