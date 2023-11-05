#include "pass_through_experiment.h"

using namespace std;

const int EXPLORE_ITERS = 500;
const int NUM_SAMPLES_PER_ITER = 10;

void PassThroughExperiment::explore_initial_activate(int& curr_node_id,
													 Problem& problem,
													 vector<ContextLayer>& context,
													 int& exit_depth,
													 int& exit_node_id,
													 RunHelper& run_helper) {
	{
		// exit
		uniform_int_distribution<int> distribution(0, this->possible_exits.size()-1);
		int rand_index = distribution(generator);
		this->curr_exit_depth = this->possible_exits[rand_index].first;
		this->curr_exit_node_id = this->possible_exits[rand_index].second;
	}

	{
		// new path
		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 2);
		geometric_distribution<int> geometric_distribution(0.5);
		if (this->curr_exit_depth == 0
				&& this->curr_exit_node_id == curr_node_id) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}
	
		uniform_int_distribution<int> type_distribution(0, 1);
		uniform_int_distribution<int> action_distribution(0, 2);
		uniform_int_distribution<int> next_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (type_distribution(generator) == 0) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);
				this->curr_actions.push_back(new ActionNode());
				this->curr_actions.back()->action = Action(action_distribution(generator));
				this->curr_sequences.push_back(NULL);

				problem.perform_action(this->curr_actions.back()->action);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_SEQUENCE);
				this->curr_actions.push_back(NULL);

				Scope* containing_scope = solution->scopes[this->scope_context[0]];
				while (true) {
					if (containing_scope->child_scopes.size() > 0 && next_distribution(generator) == 0) {
						uniform_int_distribution<int> child_distribution(0, (int)containing_scope->child_scopes.size()-1);
						containing_scope = containing_scope->child_scopes[child_distribution(generator)];
					} else {
						break;
					}
				}
				Sequence* new_sequence = create_sequence(problem,
														 context,
														 (int)this->scope_context.size(),
														 containing_scope,
														 run_helper);
				/**
				 * - falling part of new_sequence will be tied to initial instance
				 *   - but OK as there may not be a branch, and even if so, only minor impact
				 *     - doesn't impact state
				 */
				this->curr_sequences.push_back(new_sequence);
			}
		}
	}

	{
		if (this->curr_exit_depth == 0) {
			curr_node_id = this->curr_exit_node_id;
		} else {
			exit_depth = this->curr_exit_depth-1;
			exit_node_id = this->curr_exit_node_id;
		}
	}
}

void PassThroughExperiment::explore_activate(int& curr_node_id,
											 Problem& problem,
											 vector<ContextLayer>& context,
											 int& exit_depth,
											 int& exit_node_id,
											 RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->curr_actions[s_index]);
			this->curr_actions[s_index]->experiment_activate(
				problem,
				context,
				action_node_history);
			delete action_node_history;
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->curr_sequences[s_index]);
			this->curr_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->curr_exit_depth == 0) {
		curr_node_id = this->curr_exit_node_id;
	} else {
		exit_depth = this->curr_exit_depth-1;
		exit_node_id = this->curr_exit_node_id;
	}
}

void PassThroughExperiment::explore_backprop(double target_val) {
	this->curr_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_score /= NUM_SAMPLES_PER_ITER;
		if (this->curr_score > this->best_score) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_sequences[s_index];
				}
			}

			this->best_score = curr_score;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_sequences = this->curr_sequences;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_node_id = this->curr_exit_node_id;

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_sequences.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else {
					delete this->curr_sequences[s_index];
				}
			}

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_sequences.clear();
		}

		this->state_iter++;
		this->sub_state_iter = 0;
		if (this->state_iter >= EXPLORE_ITERS) {
			cout << "PassThrough" << endl;
			cout << "this->best_surprise: " << this->best_score << endl;
			if (this->best_score > 0.0) {
				Scope* containing_scope = solution->scopes[this->scope_context.back()];
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;
					} else {
						this->best_sequences[s_index]->scope_node_id = containing_scope->node_counter;
						containing_scope->node_counter++;

						this->best_sequences[s_index]->scope->id = solution->scope_counter;
						solution->scope_counter++;
					}
				}

				cout << "this->scope_context:" << endl;
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					cout << c_index << ": " << this->scope_context[c_index] << endl;
				}
				cout << "this->node_context:" << endl;
				for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
					cout << c_index << ": " << this->node_context[c_index] << endl;
				}
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index]->action.to_string();
					} else {
						cout << " S";
					}
				}
				cout << endl;

				cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
				cout << "this->best_exit_node_id: " << this->best_exit_node_id << endl;

				this->o_target_val_histories.reserve(solution->curr_num_datapoints);

				// reserve at least solution->curr_num_datapoints
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_SCORE;
				this->state_iter = 0;
			} else {
				this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
			}
		}
	}
}