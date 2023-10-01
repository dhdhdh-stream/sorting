#include "branch_experiment.h"

using namespace std;

const int EXPLORE_ITERS = 1000;

const int EXPERIMENT_SURPRISE_THRESHOLD = 1.0;
/**
 * - if surprise isn't better than what can be expected from random fluctuation, don't bother
 */

void BranchExperiment::explore_activate(int& curr_node_id,
										vector<double>& flat_vals,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double predicted_score = parent_scope->average_score;
	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network == NULL) {
			predicted_score += it->second.val
				* it->first->resolved_standard_deviation
				* it->first->scale->weight;
		} else if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			predicted_score += normalized * it->first->scale->weight;
		} else {
			predicted_score += it->second.val * it->first->scale->weight;
		}
	}
	history->existing_predicted_score = predicted_score;

	{
		// new path
		geometric_distribution<int> geometric_distribution(0.3);
		int new_num_steps = geometric_distribution(generator);
		
		uniform_int_distribution<int> type_distribution(0, 1);
		uniform_int_distribution<int> direction_distribution(0, 1);
		uniform_int_distribution<int> next_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (type_distribution(generator) == 0) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);
				this->curr_actions.push_back(new ActionNode());
				this->curr_sequences.push_back(NULL);

				flat_vals.erase(flat_vals.begin());
			} else {
				this->curr_step_types.push_back(STEP_TYPE_SEQUENCE);
				this->curr_actions.push_back(NULL);

				Scope* containing_scope;
				if (direction_distribution(generator) == 0) {
					// higher
					int context_index = context.size() - this->scope_context.size();
					while (true) {
						if (context_index > 0 && next_distribution(generator) == 0) {
							context_index--;
						} else {
							break;
						}
					}
					containing_scope = solution->scopes[context[context_index].scope_id];
				} else {
					// lower
					containing_scope = solution->scopes[this->scope_context[0]];
					while (true) {
						if (containing_scope->child_scopes.size() > 0 && next_distribution(generator) == 0) {
							uniform_int_distribution<int> child_distribution(0, containing_scope->child_scopes.size()-1);
							containing_scope = containing_scope->child_scopes[child_distribution(generator)];
						} else {
							break;
						}
					}
				}

				Sequence* new_sequence = solution->construct_sequence(flat_vals,
																	  context,
																	  this->scope_context.size(),
																	  containing_scope,
																	  run_helper);
				this->curr_sequences.push_back(new_sequence);
			}
		}
	}

	{
		// exit
		int new_exit_depth;
		int new_exit_node_id;
		solution->random_exit(this->scope_context,
							  this->node_context,
							  new_exit_depth,
							  new_exit_node_id);

		this->curr_exit_depth = new_exit_depth;
		this->curr_exit_node_id = new_exit_node_id;

		if (this->curr_exit_depth == 0) {
			curr_node_id = this->curr_exit_node_id;
		} else {
			exit_depth = this->curr_exit_depth;
			exit_node_id = this->curr_exit_node_id;
		}
	}
}

void BranchExperiment::explore_backprop(double target_val,
										BranchExperimentHistory* history) {
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double curr_surprise = (target_val - history->existing_predicted_score)
		/ parent_scope->average_misguess;
	if (curr_surprise > this->best_surprise) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->best_actions[s_index];
			} else {
				delete this->best_sequences[s_index];
			}
		}

		this->best_surprise = curr_surprise;
		this->best_step_types = this->curr_step_types;
		this->best_actions = this->curr_actions;
		this->best_sequences = this->curr_sequences;
		this->best_exit_depth = this->curr_exit_depth;
		this->best_exit_node_id = this->curr_exit_node_id;

		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_sequences.clear();
	} else {
		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->curr_actions[s_index];
			} else {
				delete this->curr_sequences[s_index]
			}
		}

		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_sequences.clear();
	}

	this->state_iter++;
	if (this->state_iter >= EXPLORE_ITERS) {
		if (this->best_surprise > EXPERIMENT_SURPRISE_THRESHOLD) {
			this->state = BRANCH_EXPERIMENT_STATE_TRAIN;
			this->state_iter = 0;
		} else {
			// if no explore passed EXPERIMENT_SURPRISE_THRESHOLD, then nothing saved/nothing to delete
			this->state = BRANCH_EXPERIMENT_STATE_DONE;
		}
	}
}
