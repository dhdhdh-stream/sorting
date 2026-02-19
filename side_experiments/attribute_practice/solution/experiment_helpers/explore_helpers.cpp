#include "experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "build_network.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void Experiment::explore_check_activate(SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	this->num_instances_until_target--;
	if (history->existing_predicted.size() == 0
			&& this->num_instances_until_target <= 0) {
		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		this->curr_exit_next_node = possible_exits[random_index];

		uniform_int_distribution<int> new_scope_distribution(0, 3);
		if (new_scope_distribution(generator) == 0) {
			this->curr_new_scope = create_new_scope(this->node_context->parent);
		}
		if (this->curr_new_scope != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_SCOPE);
			this->curr_actions.push_back(-1);
			this->curr_scopes.push_back(this->curr_new_scope);
		} else {
			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (random_index == 0) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
				if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					this->curr_scopes.push_back(this->node_context->parent->child_scopes[child_index]);
				} else {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					this->curr_actions.push_back(-1);

					this->curr_scopes.push_back(NULL);
				}
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = this->scope_context;
				new_action_node->id = this->scope_context->node_counter + s_index;

				this->curr_new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = this->scope_context;
				new_scope_node->id = this->scope_context->node_counter + s_index;

				new_scope_node->scope = this->curr_scopes[s_index];

				this->curr_new_nodes.push_back(new_scope_node);
			}
		}

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void Experiment::explore_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  bool& fetch_action,
							  SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

		history->stack_traces.push_back(wrapper->scope_histories);

		this->existing_network->activate(obs);
		history->existing_predicted.push_back(
			this->existing_network->output->acti_vals[0]);
	}

	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->curr_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->curr_new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			history->index = (int)scope_history->node_histories.size();
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::explore_set_action(int action,
									SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	this->curr_actions[experiment_state->step_index] = action;

	ActionNode* action_node = (ActionNode*)this->curr_new_nodes[experiment_state->step_index];
	action_node->action = action;

	experiment_state->step_index++;
}

void Experiment::explore_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::explore_backprop(double target_val,
								  SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
	this->num_instances_until_target = until_distribution(generator);

	if (history->existing_predicted.size() != 0) {
		double curr_surprise;
		if (this->signal_depth == -1) {
			curr_surprise = target_val - history->existing_predicted[0];
		} else {
			ScopeHistory* scope_history;
			if (this->signal_depth >= (int)history->stack_traces[0].size()) {
				scope_history = history->stack_traces[0][0];
			} else {
				int index = history->stack_traces[0].size()-1 - this->signal_depth;
				scope_history = history->stack_traces[0][index];
			}

			Scope* scope = scope_history->scope;

			double pre_signal = scope->pre_signal->activate(scope_history->pre_obs_history);

			vector<double> input;
			input.insert(input.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
			input.insert(input.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

			double post_signal = scope->post_signal->activate(input);

			double new_signal = post_signal - pre_signal;

			/**
			 * - quick sanity check
			 */
			double true_rel = target_val - this->existing_true;
			double signal_rel = new_signal - this->existing_signal;
			if (signal_rel < true_rel) {
				curr_surprise = numeric_limits<double>::lowest();
			} else {
				curr_surprise = new_signal - history->existing_predicted[0];
			}
		}

		for (int l_index = 0; l_index < (int)history->stack_traces[0].size(); l_index++) {
			signal_add_explore_sample(history->stack_traces[0][l_index],
									  target_val);
		}

		#if defined(MDEBUG) && MDEBUG
		if (curr_surprise > this->best_surprise || true) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			if (this->best_new_scope != NULL) {
				delete this->best_new_scope;
			}
			this->best_new_scope = this->curr_new_scope;
			this->curr_new_scope = NULL;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
			this->best_exit_next_node = this->curr_exit_next_node;
			for (int n_index = 0; n_index < (int)this->best_new_nodes.size(); n_index++) {
				delete this->best_new_nodes[n_index];
			}
			this->best_new_nodes = this->curr_new_nodes;
			this->curr_new_nodes.clear();
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();
		for (int n_index = 0; n_index < (int)this->curr_new_nodes.size(); n_index++) {
			delete this->curr_new_nodes[n_index];
		}
		this->curr_new_nodes.clear();

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise >= 0.0) {
			#endif /* MDEBUG */
				this->state = EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
