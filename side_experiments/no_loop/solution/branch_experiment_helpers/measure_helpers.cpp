#include "branch_experiment.h"

using namespace std;

const int MEASURE_ITERS = 10000;

const double MIN_SCORE_IMPACT = 0.05;

/**
 * - difference between simple_activate() is that increments branch_count
 */
void BranchExperiment::measure_activate(int& curr_node_id,
										vector<double>& flat_vals,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper) {
	double branch_score = 0.0;
	double original_score = 0.0;

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network == NULL) {
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += it->second.val
					* it->first->resolved_standard_deviation
					* scale_it->second->weight;
			}
			original_score += it->second.val
				* it->first->resolved_standard_deviation
				* it->first->scale->weight;
		} else if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
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

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].new_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].new_score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			branch_score += normalized * it->first->scale->weight;
		} else {
			branch_score += it->second.val * it->first->scale_weight;
		}
	}

	if (branch_score > original_score) {
		this->branch_count++;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->branch_experiment_simple_activate(
					flat_vals);
			} else {
				SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
				this->best_sequences[s_index]->activate(flat_vals,
														context,
														run_helper,
														sequence_history);
				delete sequence_history;
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node_id = this->best_exit_node_id;
		} else {
			exit_depth = this->best_exit_depth;
			exit_node_id = this->best_exit_node_id;
		}
	}
}

void BranchExperiment::measure_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		eval();
	}
}

void BranchExperiment::eval() {
	Scope* parent = solution->scopes[this->scope_context[0]];

	double combined_average_score = this->combined_score / MEASURE_ITERS;
	double combined_improvement = combined_average_score - parent->average_score;
	double score_standard_deviation = sqrt(parent->score_variance);
	double combined_improvement_t_score = combined_improvement
		/ score_standard_deviation / sqrt(MEASURE_ITERS);
	if (combined_improvement_t_score > 2.326) {		// >99%
		double branch_weight = this->branch_count / MEASURE_ITERS;
		if (branch_weight > 0.98) {

		} else {
			Scope* starting_scope = solution->scopes[this->scope_context.back()];
			Scope* parent_scope = solution->scopes[this->scope_context[0]];

			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_branch_node);

			new_branch_node->branch_scope_context = this->scope_context;
			new_branch_node->branch_node_context = this->node_context;
			new_branch_node->branch_node_context.back() = new_branch_node->id;

			new_branch_node->branch_is_pass_through = false;

			// TODO: add nodes first, so when states

			for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
					it != this->score_state_scales.end(); it++) {
				double original_impact = it->first->resolved_standard_deviation * it->first->scale->weight;
				double new_impact = it->first->resolved_standard_deviation * it->second->weight;
				if (abs(original_impact - new_impact) > MIN_SCORE_IMPACT*score_standard_deviation) {

				}
			}


		}
	} else {
		// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

		double score_improvement = this->average_score - parent->average_score;
		double score_improvement_t_score = score_improvement
			/ score_standard_deviation / sqrt(20000);

		double misguess_improvement = parent->average_misguess - this->average_misguess;
		double misguess_standard_deviation = sqrt(parent->misguess_variance);
		double misguess_improvement_t_score = misguess_improvement
			/ misguess_standard_deviation / sqrt(20000);

		if (score_improvement_t_score > -0.674	// 75%<
				&& misguess_improvement_t_score > 2.326) {

		} else {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->actions[s_index];
				} else {
					delete this->sequences[s_index];
				}
			}

			for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
					it != this->score_state_scales.end(); it++) {
				delete it->second;
			}

			for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
				delete this->new_score_states[s_index];
			}
		}
	}

	this->state = BRANCH_EXPERIMENT_STATE_DONE;
}
