#include "branch_experiment.h"

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
	double branch_score = this->average_score;
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double original_score = parent_scope->average_score;

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += normalized * scale_it->second->weight;
			}
			original_score += normalized * it->first->scale->weight;
		} else {
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += it->second.val * scale_it->second->weight;
			}
			original_score += it->second.val * it->first->scale->weight;
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			branch_score += normalized * it->first->scale->weight;
		} else {
			branch_score += it->second.val * it->first->scale->weight;
		}
	}

	if (branch_score > original_score) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
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
		}

		if (this->best_exit_depth == 0) {
			curr_node_id = this->best_exit_node_id;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node_id = this->best_exit_node_id;
		}
	}
}
