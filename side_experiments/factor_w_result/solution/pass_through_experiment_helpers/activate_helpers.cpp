#include "pass_through_experiment.h"

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	bool is_selected = false;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL
				&& run_helper.experiment_history->experiment == this) {
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
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
					run_helper.experiment_history = new PassThroughExperimentHistory(this);
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {
		explore_activate(curr_node,
						 problem,
						 context,
						 run_helper);
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	explore_backprop(target_val,
					 run_helper);
}
