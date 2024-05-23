#include "orientation_experiment.h"

using namespace std;

bool OrientationExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 EvalHistory* eval_history,
									 RunHelper& run_helper) {
	bool is_selected = false;
	OrientationExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_scope_history->experiment_histories.size() == 1
				&& run_helper.experiment_scope_history->experiment_histories[0]->experiment == this) {
			history = (OrientationExperimentHistory*)run_helper.experiment_scope_history->experiment_histories[0];
			is_selected = true;
		} else if (run_helper.experiment_scope_history->experiment_histories.size() == 0) {
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiment_scope_history->experiments_seen_order.size(); e_index++) {
				if (run_helper.experiment_scope_history->experiments_seen_order[e_index] == this) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					history = new OrientationExperimentHistory(this);
					run_helper.experiment_scope_history->experiment_histories.push_back(history);
					is_selected = true;
				}

				run_helper.experiment_scope_history->experiments_seen_order.push_back(this);
			}
		}
	}

	if (is_selected) {

	} else {

	}
}

void OrientationExperiment::backprop(EvalHistory* eval_history,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper) {

}
