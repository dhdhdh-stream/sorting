#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::activate(AbstractNode*& curr_node,
									Problem* problem,
									vector<ContextLayer>& context,
									int& exit_depth,
									AbstractNode*& exit_node,
									RunHelper& run_helper) {
	bool is_selected = false;
	if (run_helper.experiment_history == NULL) {
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
				if (run_helper.experiments_seen_order[e_index] == this->parent) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->parent->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					run_helper.experiment_history = new SeedExperimentOverallHistory(this->parent);
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this->parent);
			}
		}
	} else if (run_helper.experiment_history->experiment == this->parent) {
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
		if (this->is_candidate) {
			/**
			 * - instance_count already updated in parent
			 */
			bool is_target = false;
			SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
			if (!overall_history->has_target) {
				double target_probability;
				if (overall_history->instance_count > this->parent->average_instances_per_run) {
					target_probability = 0.5;
				} else {
					target_probability = 1.0 / (1.0 + 1.0 + (this->parent->average_instances_per_run - overall_history->instance_count));
				}
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < target_probability) {
					is_target = true;
				}
			}

			if (is_target) {
				overall_history->has_target = true;

				switch (this->parent->state) {
				case SEED_EXPERIMENT_STATE_FIND_FILTER:
				case SEED_EXPERIMENT_STATE_VERIFY_FILTER:
					find_activate(curr_node,
								  problem,
								  context,
								  exit_depth,
								  exit_node,
								  run_helper);
					break;
				case SEED_EXPERIMENT_STATE_FIND_GATHER:
				case SEED_EXPERIMENT_STATE_VERIFY_GATHER:
					if (this->parent->sub_state_iter == -1) {
						find_gather_activate(context,
											 run_helper);
					} else {
						if (this->parent->sub_state_iter%2 == 0) {
							// do nothing
						} else {
							non_seed_path_activate(curr_node);
						}
					}
					break;
				case SEED_EXPERIMENT_STATE_TRAIN_FILTER:
					train_filter_activate(context);
					break;
				case SEED_EXPERIMENT_STATE_MEASURE_FILTER:
					measure_filter_activate(curr_node,
											problem,
											context,
											exit_depth,
											exit_node,
											run_helper);
					break;
				}
			}
		} else {
			non_candidate_activate(curr_node,
								   context);
		}
	}
}
