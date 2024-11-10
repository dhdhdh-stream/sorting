#include "pass_through_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "branch_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

bool PassThroughExperiment::result_activate(
		AbstractNode* experiment_node,
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
					run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(this));
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	}

	return false;
}

bool PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper) {
	bool is_selected = false;
	PassThroughExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = (PassThroughExperimentHistory*)run_helper.experiment_histories[match_index];
			is_selected = true;
		}
	}

	if (is_selected) {
		explore_activate(curr_node,
						 problem,
						 context,
						 run_helper,
						 history);

		return true;
	} else {
		return false;
	}
}

void PassThroughExperiment::split_activate(BranchNode* branch_node,
										   bool existing_is_branch,
										   Problem* problem,
										   vector<ContextLayer>& context,
										   RunHelper& run_helper) {
	uniform_real_distribution<double> split_distribution(0.0, solution->average_num_analyze);
	if (split_distribution(generator) <= 2.0) {
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

		this->curr_existing_impact += (run_helper.result - target_val)
			- branch_node->impact;

		delete copy_problem;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	explore_backprop(target_val,
					 run_helper);
}
