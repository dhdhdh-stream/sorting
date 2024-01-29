#include "scenario_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "solution.h"
#include "solution_helpers.h"
#include "scenario.h"
#include "scope.h"

using namespace std;

const int LEARN_ITERS = 40;

ScenarioExperiment::ScenarioExperiment(Scenario* scenario_type) {
	this->scenario_type = scenario_type;

	this->scope = new Scope();

	this->scope->id = solution->scope_counter;
	solution->scope_counter++;

	this->scope->name = this->scenario_type->get_name();

	this->scope->num_input_states = 0;
	this->scope->num_local_states = 0;
	this->scope->node_counter = 0;

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->parent = this->scope;
	new_noop_action_node->id = this->scope->node_counter;
	this->scope->node_counter++;
	new_noop_action_node->action = Action(ACTION_NOOP);
	this->scope->nodes[new_noop_action_node->id] = new_noop_action_node;

	this->scope->starting_node_id = new_noop_action_node->id;
	this->scope->starting_node = new_noop_action_node;

	vector<int> types;
	vector<Action> actions;
	vector<string> scopes;
	this->scenario_type->get_attention(types,
									   actions,
									   scopes);

	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		ActionNode* new_action_node = new ActionNode();

		new_action_node->parent = this->scope;
		new_action_node->id = this->scope->node_counter;
		this->scope->node_counter++;
		this->scope->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = actions[a_index];
	}

	for (int n_index = 0; n_index < (int)this->scope->nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)this->scope->nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = this->scope->nodes[n_index+1];
		}

		ActionNode* action_node = (ActionNode*)this->scope->nodes[n_index];
		action_node->next_node_id = next_node_id;
		action_node->next_node = next_node;
	}

	this->state = SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE;
	this->state_iter = 0;
	this->sub_state_iter = 0;
}

ScenarioExperiment::~ScenarioExperiment() {
	delete this->scenario_type;
}

void ScenarioExperiment::activate(Scenario* scenario,
								  RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(this->scope);
	context.back().scope_history = root_history;

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	this->scope->activate(scenario->problem,
						  context,
						  exit_depth,
						  exit_node,
						  run_helper,
						  root_history);

	if (this->state == SCENARIO_EXPERIMENT_STATE_LEARN) {
		this->scope_histories.push_back(new ScopeHistory(root_history));

		double predicted_is_sequence = this->is_sequence_average;
		for (int s_index = 0; s_index < this->scope->num_local_states; s_index++) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(s_index);
			if (it != context.back().local_state_vals.end()) {
				predicted_is_sequence += it->second.val;
			}
		}
		this->predicted_is_sequence_histories.push_back(predicted_is_sequence);
	}

	delete root_history;
}

void ScenarioExperiment::backprop(bool is_sequence) {
	this->is_sequence_histories.push_back(is_sequence);

	if ((int)this->is_sequence_histories.size() >= solution->curr_num_datapoints) {
		if (this->state == SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE) {
			int is_sequence_count = 0;
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				if (this->is_sequence_histories[i_index]) {
					is_sequence_count++;
				}
			}
			this->is_sequence_average = 2.0 * (double)is_sequence_count / (double)solution->curr_num_datapoints - 1.0;

			this->is_sequence_histories.clear();

			this->state = SCENARIO_EXPERIMENT_STATE_LEARN;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			// cout << "this->sub_state_iter: " << this->sub_state_iter << endl;
			// double sum_misguess = 0.0;
			// for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
			// 	if (this->is_sequence_histories[i_index]) {
			// 		sum_misguess += abs(1.0 - this->predicted_is_sequence_histories[i_index]);
			// 	} else {
			// 		sum_misguess += abs(-1.0 - this->predicted_is_sequence_histories[i_index]);
			// 	}
				
			// }
			// cout << "sum_misguess: " << sum_misguess << endl;

			vector<double> obs_experiment_target_vals(solution->curr_num_datapoints);
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				if (this->is_sequence_histories[i_index]) {
					obs_experiment_target_vals[i_index] = 1.0 - this->predicted_is_sequence_histories[i_index];
				} else {
					obs_experiment_target_vals[i_index] = -1.0 - this->predicted_is_sequence_histories[i_index];
				}
			}

			scenario_obs_experiment(this,
									this->scope_histories,
									obs_experiment_target_vals);

			for (int i_index = 0; i_index < (int)this->scope_histories.size(); i_index++) {
				delete this->scope_histories[i_index];
			}
			this->scope_histories.clear();
			this->predicted_is_sequence_histories.clear();
			this->is_sequence_histories.clear();

			this->sub_state_iter++;
			if (this->sub_state_iter > LEARN_ITERS) {
				ActionNode* attention_last_node = (ActionNode*)this->scope->nodes[this->scope->nodes.size()-1];

				BranchNode* new_branch_node = new BranchNode();
				new_branch_node->parent = this->scope;
				new_branch_node->id = this->scope->node_counter;
				this->scope->node_counter++;
				this->scope->nodes[new_branch_node->id] = new_branch_node;

				new_branch_node->branch_scope_context = vector<int>{this->scope->id};
				new_branch_node->branch_node_context = vector<int>{new_branch_node->id};

				new_branch_node->branch_is_pass_through = false;

				new_branch_node->original_score_mod = 0.0;
				new_branch_node->branch_score_mod = this->is_sequence_average;

				for (int s_index = 0; s_index < this->scope->num_local_states; s_index++) {
					new_branch_node->decision_state_is_local.push_back(true);
					new_branch_node->decision_state_indexes.push_back(s_index);
					new_branch_node->decision_original_weights.push_back(0.0);
					new_branch_node->decision_branch_weights.push_back(1.0);
				}

				/**
				 * - simply set to 0.5
				 */
				new_branch_node->decision_standard_deviation = 0.5;

				attention_last_node->next_node_id = new_branch_node->id;
				attention_last_node->next_node = new_branch_node;

				vector<int> types;
				vector<Action> actions;
				vector<string> scopes;
				this->scenario_type->get_sequence(types,
												  actions,
												  scopes);

				for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
					ActionNode* new_action_node = new ActionNode();

					new_action_node->parent = this->scope;
					new_action_node->id = this->scope->node_counter;
					this->scope->node_counter++;
					this->scope->nodes[new_action_node->id] = new_action_node;

					new_action_node->action = actions[a_index];
				}

				new_branch_node->original_next_node_id = -1;
				new_branch_node->original_next_node = NULL;

				new_branch_node->branch_next_node_id = new_branch_node->id + 1;
				new_branch_node->branch_next_node = this->scope->nodes[new_branch_node->id + 1];

				for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
					int next_node_id;
					AbstractNode* next_node;
					if (a_index == (int)actions.size()-1) {
						next_node_id = -1;
						next_node = NULL;
					} else {
						next_node_id = new_branch_node->id + 1 + a_index + 1;
						next_node = this->scope->nodes[new_branch_node->id + 1 + a_index + 1];
					}

					ActionNode* action_node = (ActionNode*)this->scope->nodes[new_branch_node->id + 1 + a_index];
					action_node->next_node_id = next_node_id;
					action_node->next_node = next_node;
				}

				solution->scopes[this->scope->id] = this->scope;

				this->state = SCENARIO_EXPERIMENT_STATE_DONE;
			}
			// else continue
		}
	}
}
