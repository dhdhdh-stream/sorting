#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

const int BRANCH_EXPERIMENT_STATE_EXISTING_GATHER = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING = 1;
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 2;
const int BRANCH_EXPERIMENT_STATE_NEW_GATHER = 3;
const int BRANCH_EXPERIMENT_STATE_TRAIN_NEW = 4;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 5;
#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY = 6;
#endif /* MDEBUG */

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> existing_inputs;

	double existing_average_score;
	std::vector<std::pair<int,int>> existing_factor_ids;
	std::vector<double> existing_factor_weights;

	int num_instances_until_target;

	int explore_type;

	std::vector<int> curr_step_types;
	std::vector<Action*> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_surprise;
	std::vector<int> best_step_types;
	std::vector<Action*> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> new_inputs;

	std::vector<std::pair<int,int>> new_factor_ids;
	std::vector<double> new_factor_weights;

	std::vector<std::vector<double>> input_histories;
	std::vector<std::map<std::pair<int,int>, double>> factor_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> o_target_val_histories;

};

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;
	std::vector<double> existing_predicted_scores;

	BranchExperimentHistory(BranchExperiment* experiment);
};

#endif /* BRANCH_EXPERIMENT_H */