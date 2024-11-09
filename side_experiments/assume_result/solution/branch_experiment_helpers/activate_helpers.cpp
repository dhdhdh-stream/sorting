#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

bool BranchExperiment::result_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper) {
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_histories.size() == 0) {
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				if (run_helper.experiments_seen_order[e_index] == this) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					run_helper.experiment_histories.push_back(new BranchExperimentHistory(this));
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	}

	return false;
}

bool BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	bool is_selected = false;
	BranchExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = (BranchExperimentHistory*)run_helper.experiment_histories[match_index];
			is_selected = true;
		}
	}

	bool result = false;
	if (is_selected) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			result = explore_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			result = train_new_activate(curr_node,
										problem,
										context,
										run_helper,
										history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			result = measure_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			result = capture_verify_activate(curr_node,
											 problem,
											 context,
											 run_helper);
			break;
		#endif /* MDEBUG */
		}
	}

	return result;
}

void BranchExperiment::split_activate(BranchNode* branch_node,
									  bool existing_is_branch,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper) {
	if (this->state == BRANCH_EXPERIMENT_STATE_MEASURE) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

		Problem* copy_problem = problem->copy_snapshot();

		RunHelper copy_run_helper = run_helper;
		copy_run_helper.is_split = true;

		vector<ContextLayer> copy_context = context;
		if (existing_is_branch) {
			copy_context.back().node = branch_node->original_next_node;
		} else {
			copy_context.back().node = branch_node->branch_next_node;
		}
		solution->scopes[0]->continue_experiment_activate(
			copy_problem,
			copy_context,
			0,
			copy_run_helper);

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = copy_problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		} else {
			target_val = -1.0;
		}

		history->existing_impacts.push_back(branch_node->impact);
		history->new_impacts.push_back(run_helper.result - target_val);

		delete copy_problem;
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
