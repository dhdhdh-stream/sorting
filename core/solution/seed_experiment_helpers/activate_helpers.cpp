#include "seed_experiment.h"

using namespace std;

void SeedExperiment::activate() {
	bool is_selected = false;
	if (this->parent_pass_through_experiment != NULL) {
		is_selected = true;
	} else if (run_helper.experiment_history == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
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
					run_helper.experiment_history = new SeedExperimentOverallHistory(this);
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
		}
	} else if (run_helper.experiment_history->experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case SEED_EXPERIMENT_STATE_TRAIN_EXISTING:

			break;
		case SEED_EXPERIMENT_STATE_EXPLORE:

			break;
		default:
			/**
			 * - safe to always take seed path
			 *   - in non-target cases, will be filtered by curr_filter
			 *   - likely overridden by starting filter anyways
			 */
			if (this->best_step_types.size() == 0) {
				if (this->best_exit_depth == 0) {
					curr_node = 
				} else {

				}
			} else {
				if (this->best_step_types[0]->type == STEP_TYPE_ACTION) {

				} else if () {

				} else {

				}
			}

			break;
		}
	}
}

void SeedExperiment::backprop() {
	

}
