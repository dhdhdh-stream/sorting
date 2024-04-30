#include "eval_experiment.h"

using namespace std;

bool EvalExperiment::activate(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  RunHelper& run_helper) {
	bool is_selected = false;
	EvalExperimentHistory* history = NULL;
	if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories[0]->experiment == this) {
		history = (EvalExperimentHistory*)run_helper.experiment_histories[0];
		is_selected = true;
	} else if (run_helper.experiment_histories.size() == 0) {
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
				history = new EvalExperimentHistory(this);
				run_helper.experiment_histories.push_back(history);
				is_selected = true;
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}

	if (is_selected) {

	} else {

	}
}
