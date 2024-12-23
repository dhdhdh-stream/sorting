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

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	explore_backprop(target_val,
					 run_helper);
}
