#include "explore_experiment.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "pass_through_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void ExploreExperiment::measure_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		bool is_branch;
		this->existing_network->activate(wrapper->states.back());
		if (this->is_new_state) {
			vector<bool> curr_dependencies_is_hit(this->dependencies.size());
			vector<Eigen::VectorXf> curr_dependencies_state(this->dependencies.size());
			vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				bool is_hit;
				Eigen::VectorXf state;
				vector<double> obs;
				fetch_dependency_helper(scope_history,
										this->dependencies[d_index],
										0,
										is_hit,
										state,
										obs);
				curr_dependencies_is_hit[d_index] = is_hit;
				curr_dependencies_state[d_index] = state;
				curr_dependencies_obs[d_index] = obs;
			}

			vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				if (curr_dependencies_is_hit[d_index]) {
					this->measure_init_networks[d_index]->init_activate(
						curr_dependencies_state[d_index],
						new_state,
						curr_dependencies_obs[d_index]);
				}
			}
			this->measure_new_network->init_activate(wrapper->states.back(),
													 new_state);
		} else {
			this->measure_new_network->activate(wrapper->states.back());
		}
		if (this->measure_new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		if (is_branch) {
			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::measure_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			Scope* inner_scope = this->best_scopes[experiment_state->step_index];
			ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(inner_scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(inner_scope->num_states);
			wrapper->states.back().setConstant(0.0);
			wrapper->partial_states.push_back(Eigen::VectorXf());
			wrapper->partial_states.back().resize(inner_scope->num_states);
			wrapper->partial_states.back().setConstant(0.0);

			inner_scope->experiment_start_activate(
				obs,
				wrapper);
		}
	}
}

void ExploreExperiment::measure_callback(vector<double>& obs,
										 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	int action = this->best_actions[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];

	generic_action_node->action_network->activate(wrapper->states.back());

	generic_action_node->obs_network->activate(wrapper->states.back(),
											   obs);

	experiment_state->step_index++;
}

void ExploreExperiment::measure_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();
	wrapper->partial_states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::measure_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		this->sum_vals += target_val;

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
			double new_val_average = this->sum_vals / this->state_iter;
			cout << "new_val_average: " << new_val_average << endl;

			if (this->is_new_state) {
				set<Scope*> scopes_needed;
				set<ScopeNode*> transitions_needed;
				scopes_needed.insert(this->scope_context);
				for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
					get_dependency_changes_helper(this->scope_context,
												  this->dependencies[d_index],
												  0,
												  scopes_needed,
												  transitions_needed);
				}

				for (set<ScopeNode*>::iterator it = transitions_needed.begin();
						it != transitions_needed.end(); it++) {
					ScopeNode* scope_node = *it;
					int out_num_states = scope_node->parent->num_states;
					int in_num_states = scope_node->scope->num_states;
					for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
						PassThroughNetwork* new_in_pass_through_network = new PassThroughNetwork(
							out_num_states + s_index,
							in_num_states + s_index);
						scope_node->in_pass_through_networks.push_back(new_in_pass_through_network);

						PassThroughNetwork* new_out_pass_through_network = new PassThroughNetwork(
							in_num_states + s_index,
							out_num_states + s_index);
						scope_node->out_pass_through_networks.push_back(new_out_pass_through_network);
					}
				}

				for (set<Scope*>::iterator it = scopes_needed.begin();
						it != scopes_needed.end(); it++) {
					Scope* scope = *it;

					scope->num_states += NEW_STATE_NUM_ADD;

					scope->start_obs_network->add_states(scope->num_states);
					for (int n_index = 0; n_index < (int)scope->start_init_networks.size(); n_index++) {
						scope->start_init_networks[n_index]->add_states(scope->num_states);
					}
					scope->end_score_network->add_states(scope->num_states);

					for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
							it != scope->nodes.end(); it++) {
						switch (it->second->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)it->second;

								action_node->action_network->add_states(scope->num_states);
								action_node->obs_network->add_states(scope->num_states);
								for (int n_index = 0; n_index < (int)action_node->init_networks.size(); n_index++) {
									action_node->init_networks[n_index]->add_states(scope->num_states);
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)it->second;

								scope_node->in_network->add_front_states(scope->num_states);
								scope_node->out_network->add_back_states(scope->num_states);
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)it->second;

								branch_node->original_network->add_states(scope->num_states);
								branch_node->branch_network->add_states(scope->num_states);
							}
							break;
						}
					}

					for (int a_index = 0; a_index < (int)scope->generic_action_nodes.size(); a_index++) {
						ActionNode* action_node = scope->generic_action_nodes[a_index];

						action_node->action_network->add_states(scope->num_states);
						action_node->obs_network->add_states(scope->num_states);
					}

					for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
						Scope* p_outer_scope = wrapper->solution->scopes[s_index];
						for (map<int, AbstractNode*>::iterator it = p_outer_scope->nodes.begin();
								it != p_outer_scope->nodes.end(); it++) {
							if (it->second->type == NODE_TYPE_SCOPE) {
								ScopeNode* scope_node = (ScopeNode*)it->second;
								if (scope_node->scope == scope) {
									scope_node->in_network->add_back_states(scope->num_states);
									scope_node->out_network->add_front_states(scope->num_states);
								}
							}
						}
					}
				}

				for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
					vector<Scope*> init_network_scope_context;
					add_dependency_helper(this->scope_context,
										  init_network_scope_context,
										  this->dependencies[d_index],
										  0,
										  this->measure_init_networks[d_index]);
				}

				this->existing_network->add_states(this->scope_context->num_states);

				add(true,
					this->measure_new_network,
					wrapper);
			} else {
				add(false,
					this->measure_new_network,
					wrapper);
			}

			delete this;

			for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
				Scope* scope = wrapper->solution->scopes[s_index];
				for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
						it != scope->nodes.end(); it++) {
					switch (it->second->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)it->second;
							if (noop_node->experiment != NULL) {
								delete noop_node->experiment;
							}
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->second;
							if (action_node->experiment != NULL) {
								delete action_node->experiment;
							}
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)it->second;
							if (scope_node->experiment != NULL) {
								delete scope_node->experiment;
							}
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)it->second;
							if (branch_node->original_experiment != NULL) {
								delete branch_node->original_experiment;
							}
							if (branch_node->branch_experiment != NULL) {
								delete branch_node->branch_experiment;
							}
						}
						break;
					}
				}
			}

			wrapper->experiment_iter = 0;
		}
	}
}
