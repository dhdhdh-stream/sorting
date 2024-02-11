#ifndef RETRAIN_BRANCH_EXPERIMENT_H
#define RETRAIN_BRANCH_EXPERIMENT_H

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
	int original_count;
	int branch_count;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<double> i_target_val_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

};

class RetrainBranchExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;

	RetrainBranchExperimentOverallHistory(RetrainBranchExperiment* experiment);
};

#endif /* RETRAIN_BRANCH_EXPERIMENT_H */