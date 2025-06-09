#include "commit_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void CommitExperiment::commit_existing_gather_check_activate(
		SolutionWrapper* wrapper) {
	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
	new_experiment_state->is_save = false;
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CommitExperiment::commit_existing_gather_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
			wrapper->node_context.back() = this->save_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				action = this->save_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
				wrapper->confusion_context.push_back(NULL);

				if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
					this->save_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
				}
			}
		}
	} else {
		if (experiment_state->step_index >= this->step_iter) {
			vector<Scope*> scope_context;
			vector<int> node_context;
			int node_count = 0;
			Input new_input;
			gather_possible_helper(wrapper->scope_histories.back(),
								   scope_context,
								   node_context,
								   node_count,
								   new_input,
								   wrapper);

			bool is_existing = false;
			for (int i_index = 0; i_index < (int)this->commit_existing_inputs.size(); i_index++) {
				if (new_input == this->commit_existing_inputs[i_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->commit_existing_inputs.push_back(new_input);
			}

			for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
				pair<int,int> new_factor = {-1, -1};
				gather_possible_factor_helper(wrapper->scope_histories.back(),
											  new_factor);

				if (new_factor.first != -1) {
					bool is_existing = false;
					for (int i_index = 0; i_index < (int)this->commit_existing_factor_ids.size(); i_index++) {
						if (new_factor == this->commit_existing_factor_ids[i_index]) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						this->commit_existing_factor_ids.push_back(new_factor);
					}
				}
			}

			experiment_state->is_save = true;
			experiment_state->step_index = 0;
		} else {
			switch (this->new_nodes[experiment_state->step_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

					action = node->action;
					is_next = true;

					wrapper->num_actions++;

					experiment_state->step_index++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ScopeNodeHistory* history = new ScopeNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
					history->scope_history = inner_scope_history;
					wrapper->scope_histories.push_back(inner_scope_history);
					wrapper->node_context.push_back(node->scope->nodes[0]);
					wrapper->experiment_context.push_back(NULL);
					wrapper->confusion_context.push_back(NULL);

					if (node->scope->new_scope_experiment != NULL) {
						node->scope->new_scope_experiment->pre_activate(wrapper);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ObsNodeHistory* history = new ObsNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					history->obs_history = obs;

					history->factor_initialized = vector<bool>(node->factors.size(), false);
					history->factor_values = vector<double>(node->factors.size());

					experiment_state->step_index++;
				}
				break;
			}
		}
	}
}

void CommitExperiment::commit_existing_gather_exit_step(
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	} else {
		ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

		if (node->scope->new_scope_experiment != NULL) {
			node->scope->new_scope_experiment->back_activate(wrapper);
		}

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	}
}

void CommitExperiment::commit_existing_gather_backprop() {
	this->state_iter++;
	if (this->state_iter >= GATHER_ITERS) {
		while (this->commit_existing_inputs.size() > GATHER_ITERS) {
			uniform_int_distribution<int> remove_distribution(0, this->commit_existing_inputs.size()-1);
			int remove_index = remove_distribution(generator);
			this->commit_existing_inputs.erase(this->commit_existing_inputs.begin() + remove_index);
		}

		while (this->commit_existing_factor_ids.size() > GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
			uniform_int_distribution<int> remove_distribution(0, this->commit_existing_factor_ids.size()-1);
			int remove_index = remove_distribution(generator);
			this->commit_existing_factor_ids.erase(this->commit_existing_factor_ids.begin() + remove_index);
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING;
		this->state_iter = 0;
	}
}
