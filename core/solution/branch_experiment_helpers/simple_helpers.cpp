#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::simple_activate(int& curr_node_id,
									   Problem& problem,
									   vector<ContextLayer>& context,
									   int& exit_depth,
									   int& exit_node_id,
									   RunHelper& run_helper) {
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	double original_score = this->existing_average_score;
	double branch_score = this->new_average_score;

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		double normalized = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;

		map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.find(it->first);
		if (existing_it != this->existing_starting_score_state_scales.end()) {
			original_score += existing_it->second->weight * normalized;
		}
		map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(it->first);
		if (new_it != this->new_starting_score_state_scales.end()) {
			branch_score += new_it->second->weight * normalized;
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_score_state_vals.end(); it++) {
		map<State*, Scale*>::iterator scale_it = this->new_starting_experiment_score_state_scales.find(it->first);
		if (scale_it != this->new_starting_experiment_score_state_scales.end()) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			branch_score += scale_it->second->weight * normalized;
		}
	}

	if (branch_score > original_score) {
		// leave context.back().node_id as -1

		context.push_back(ContextLayer());

		context.back().scope_id = this->new_scope_id;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			context.back().node_id = 1 + s_index;

			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->branch_experiment_simple_activate(
					problem);
			} else {
				SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
				this->best_sequences[s_index]->activate(problem,
														context,
														run_helper,
														sequence_history);
				delete sequence_history;
			}

			// don't need to worry about run_helper.node_index

			context.back().node_id = -1;
		}

		context.pop_back();

		if (this->best_exit_depth == 0) {
			curr_node_id = this->best_exit_node_id;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node_id = this->best_exit_node_id;
		}
	}

	run_helper.curr_depth--;
}
