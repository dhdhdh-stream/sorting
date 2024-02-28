#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool SeedExperiment::activate(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper,
							  AbstractExperimentHistory*& history) {
	if (context.back().scope_history->node_histories.size() > 1000) {
		cout << "SeedExperiment" << endl;
		cout << "this->state: " << this->state << endl;
		throw invalid_argument("context.back().scope_history->node_histories.size() > 1000");
	}

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
			train_existing_activate(context,
									run_helper);
			break;
		case SEED_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper);
			break;
		case SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
		case SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
			// do nothing
			break;
		default:
			#if defined(MDEBUG) && MDEBUG
			if (this->state == SEED_EXPERIMENT_STATE_CAPTURE_VERIFY) {
				if (this->verify_problems[this->state_iter] == NULL) {
					this->verify_problems[this->state_iter] = problem->copy_and_reset();
				}
				this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;
			}
			#endif /* MDEBUG */

			/**
			 * - safe to always take seed path
			 *   - in non-target cases, will be filtered by curr_filter
			 *   - likely overridden by starting filter anyways
			 */
			if (this->best_step_types.size() == 0) {
				if (this->best_exit_depth == 0) {
					curr_node = this->best_exit_next_node;
				} else {
					curr_node = this->best_exit_node;
				}
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
					curr_node = this->best_existing_scopes[0];
				} else {
					curr_node = this->best_potential_scopes[0];
				}
			}

			break;
		}

		return true;
	} else {
		return false;
	}
}

void SeedExperiment::backprop(double target_val,
							  RunHelper& run_helper,
							  AbstractExperimentHistory* history) {
	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)history;

	switch (this->state) {
	case SEED_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								overall_history);
		break;
	case SEED_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 overall_history);
		break;
	case SEED_EXPERIMENT_STATE_FIND_FILTER:
		find_filter_backprop(target_val,
							 run_helper,
							 overall_history);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER:
	case SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER:
		verify_filter_backprop(target_val,
							   overall_history);
		break;
	case SEED_EXPERIMENT_STATE_FIND_GATHER:
		find_gather_backprop(target_val,
							 run_helper,
							 overall_history);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER:
	case SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER:
		verify_gather_backprop(target_val,
							   overall_history);
		break;
	case SEED_EXPERIMENT_STATE_TRAIN_FILTER:
		train_filter_backprop(target_val,
							  overall_history);
		break;
	case SEED_EXPERIMENT_STATE_MEASURE_FILTER:
		measure_filter_backprop(target_val,
								overall_history);
		break;
	case SEED_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper,
								 overall_history);
		break;
	case SEED_EXPERIMENT_STATE_VERIFY_1ST:
	case SEED_EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(target_val);
		break;
	#if defined(MDEBUG) && MDEBUG
	case SEED_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
