#ifndef RETRAIN_BRANCH_EXPERIMENT_H
#define RETRAIN_BRANCH_EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"
#include "state_status.h"

class BranchNode;
class Problem;
class ScopeHistory;
class State;

const int RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_ORIGINAL = 1;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_BRANCH = 2;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE = 3;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING = 4;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY = 5;
#if defined(MDEBUG) && MDEBUG
const int RETRAIN_BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 6;
#endif /* MDEBUG */

const int RETRAIN_BRANCH_EXPERIMENT_STATE_FAIL = 7;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_SUCCESS = 8;

class RetrainBranchExperimentOverallHistory;
class RetrainBranchExperiment : public AbstractExperiment {
public:
	BranchNode* branch_node;

	double average_instances_per_run;

	int state;
	int state_iter;

	double existing_average_score;
	double existing_score_variance;
	double existing_standard_deviation;

	double original_average_score;

	std::vector<std::map<int, double>> original_input_state_weights;
	std::vector<std::map<int, double>> original_local_state_weights;
	std::vector<std::map<State*, double>> original_temp_state_weights;

	double branch_average_score;

	std::vector<std::map<int, double>> branch_input_state_weights;
	std::vector<std::map<int, double>> branch_local_state_weights;
	std::vector<std::map<State*, double>> branch_temp_state_weights;

	double combined_score;

	double verify_existing_average_score;
	double verify_existing_score_variance;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	std::vector<std::vector<double>> verify_factors;
	#endif /* MDEBUG */

	RetrainBranchExperiment(BranchNode* branch_node);
	~RetrainBranchExperiment();

	bool activate(bool& is_branch,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  RetrainBranchExperimentOverallHistory* history);

	void train_original_activate(bool& is_branch,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void train_original_target_activate(bool& is_branch,
										std::vector<ContextLayer>& context);
	void train_original_non_target_activate(bool& is_branch,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	void train_original_backprop(double target_val,
								 RetrainBranchExperimentOverallHistory* history);

	void train_branch_activate(bool& is_branch,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper);
	void train_branch_target_activate(bool& is_branch,
									  std::vector<ContextLayer>& context);
	void train_branch_non_target_activate(bool& is_branch,
										  std::vector<ContextLayer>& context,
										  RunHelper& run_helper);
	void train_branch_backprop(double target_val,
							   RetrainBranchExperimentOverallHistory* history);

	void measure_existing_activate(bool& is_branch,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void measure_activate(bool& is_branch,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);
	void measure_backprop(double target_val);

	void verify_existing_activate(bool& is_branch,
								  std::vector<ContextLayer>& context,
								  RunHelper& run_helper);
	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_activate(bool& is_branch,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void verify_backprop(double target_val);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(bool& is_branch,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize();
};

class RetrainBranchExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	RetrainBranchExperimentOverallHistory(RetrainBranchExperiment* experiment);
};

#endif /* RETRAIN_BRANCH_EXPERIMENT_H */