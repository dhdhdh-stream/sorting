#include "seed_experiment_filter.h"

#include <iostream>

#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "seed_experiment.h"

using namespace std;

bool SeedExperimentFilter::activate(AbstractNode*& curr_node,
									Problem* problem,
									vector<ContextLayer>& context,
									int& exit_depth,
									AbstractNode*& exit_node,
									RunHelper& run_helper,
									AbstractExperimentHistory*& history) {
	if (context.back().scope_history->node_histories.size() > 1000) {
		cout << "SeedExperimentFilter" << endl;
		cout << "this->parent->state: " << this->parent->state << endl;
		throw invalid_argument("context.back().scope_history->node_histories.size() > 1000");
	}

	bool is_selected = false;
	if (this->parent->state != SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& this->parent->state != SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING) {
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
	}

	if (is_selected) {
		if (this->is_candidate) {
			switch (this->parent->state) {
			case SEED_EXPERIMENT_STATE_FIND_FILTER:
			case SEED_EXPERIMENT_STATE_VERIFY_1ST_FILTER:
			case SEED_EXPERIMENT_STATE_VERIFY_2ND_FILTER:
				find_filter_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
				break;
			case SEED_EXPERIMENT_STATE_FIND_GATHER:
			case SEED_EXPERIMENT_STATE_VERIFY_1ST_GATHER:
			case SEED_EXPERIMENT_STATE_VERIFY_2ND_GATHER:
				find_gather_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
				break;
			case SEED_EXPERIMENT_STATE_TRAIN_FILTER:
				train_filter_activate(curr_node,
									  problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper);
				break;
			case SEED_EXPERIMENT_STATE_MEASURE_FILTER:
				measure_filter_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper);
				break;
			case SEED_EXPERIMENT_STATE_MEASURE:
			case SEED_EXPERIMENT_STATE_VERIFY_1ST:
			case SEED_EXPERIMENT_STATE_VERIFY_2ND:
				measure_activate(curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper);
				break;
			#if defined(MDEBUG) && MDEBUG
			case SEED_EXPERIMENT_STATE_CAPTURE_VERIFY:
				candidate_capture_verify_activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper);
				break;
			#endif /* MDEBUG */
			}
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (this->parent->state == SEED_EXPERIMENT_STATE_CAPTURE_VERIFY) {
				non_candidate_capture_verify_activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper);
			} else {
				curr_node = this->branch_node;
			}
			#else
			curr_node = this->branch_node;
			#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}