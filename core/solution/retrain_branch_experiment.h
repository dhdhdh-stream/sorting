#ifndef RETRAIN_BRANCH_EXPERIMENT_H
#define RETRAIN_BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class BranchNode;
class Network;
class Problem;
class ScopeHistory;

const int RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_ORIGINAL = 1;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_BRANCH = 2;
/**
 * - share sub_state_iter
 */
const int RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE = 3;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 4;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_1ST = 5;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 6;
const int RETRAIN_BRANCH_EXPERIMENT_STATE_VERIFY_2ND = 7;
#if defined(MDEBUG) && MDEBUG
const int RETRAIN_BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 8;
#endif /* MDEBUG */

const int RETRAIN_BRANCH_EXPERIMENT_TRAIN_ITERS = 3;

class RetrainBranchExperimentOverallHistory;
class RetrainBranchExperiment : public AbstractExperiment {
public:
	BranchNode* branch_node;

	double average_instances_per_run;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	double original_average_score;
	std::vector<double> original_linear_weights;
	std::vector<std::vector<int>> original_network_input_indexes;
	Network* original_network;
	double original_average_misguess;
	double original_misguess_variance;

	double branch_average_score;
	std::vector<double> branch_linear_weights;
	std::vector<std::vector<int>> branch_network_input_indexes;
	Network* branch_network;
	double branch_average_misguess;
	double branch_misguess_variance;

	double combined_score;
	/**
	 * TODO:
	 * - track original_count vs. branch_count and switch to pass_through if able
	 */

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	RetrainBranchExperiment(BranchNode* branch_node);
	~RetrainBranchExperiment();

	bool activate(bool& is_branch,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  AbstractExperimentHistory* history);

	void measure_existing_activate(bool& is_branch,
								   std::vector<ContextLayer>& context,
								   RunHelper& run_helper);
	void measure_existing_backprop(double target_val,
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

	// unused
	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);
};

class RetrainBranchExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	RetrainBranchExperimentOverallHistory(RetrainBranchExperiment* experiment);
};

#endif /* RETRAIN_BRANCH_EXPERIMENT_H */