#include "markov_experiment.h"

#include "globals.h"

using namespace std;

bool MarkovExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	bool is_selected = false;
	MarkovExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = (MarkovExperimentHistory*)run_helper.experiment_histories[match_index];
			is_selected = true;
		} else {
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
						history = new MarkovExperimentHistory(this);
						run_helper.experiment_histories.push_back(history);
						is_selected = true;
					}

					run_helper.experiments_seen_order.push_back(this);
				}
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case MARKOV_EXPERIMENT_STATE_TRAIN:
			train_activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history);
			break;
		case MARKOV_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);
			break;
		}
	}

	return true;
}

void MarkovExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	switch (this->state) {
	case MARKOV_EXPERIMENT_STATE_TRAIN:
		train_backprop(target_val,
					   run_helper);
		break;
	case MARKOV_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	}
}
