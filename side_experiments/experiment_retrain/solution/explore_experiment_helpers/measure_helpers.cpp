#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 20;
#else
const int MEASURE_NUM_ITERS = 2000;
#endif /* MDEBUG */

void ExploreExperiment::measure_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		double existing_predicted;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				noop_node->score_network->activate(wrapper->state);
				existing_predicted = noop_node->score_network->output->acti_vals(0);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				action_node->score_network->activate(wrapper->state);
				existing_predicted = action_node->score_network->output->acti_vals(0);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				scope_node->score_network->activate(wrapper->state);
				existing_predicted = scope_node->score_network->output->acti_vals(0);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					branch_node->branch_network->activate(wrapper->state);
					existing_predicted = branch_node->branch_network->output->acti_vals(0);
				} else {
					branch_node->original_network->activate(wrapper->state);
					existing_predicted = branch_node->original_network->output->acti_vals(0);
				}
			}
			break;
		}

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);

		for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
			bool is_hit;
			vector<double> state;
			vector<double> obs;
			fetch_dependency_helper(wrapper->scope_histories.back(),
									this->best_dependencies[d_index],
									0,
									is_hit,
									state,
									obs);

			if (is_hit) {
				this->measure_init_networks[d_index]->init_activate(state,
																	new_state,
																	obs);
			}
		}

		vector<double> combined_state;
		combined_state.insert(combined_state.end(), wrapper->state.begin(), wrapper->state.end());
		combined_state.insert(combined_state.end(), new_state.begin(), new_state.end());
		this->measure_new_network->activate(combined_state);
		double new_predicted = this->measure_new_network->output->acti_vals(0);

		if (new_predicted >= existing_predicted) {
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

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ExploreExperiment::measure_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::measure_backprop(double target_val,
										 ExploreExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->measure_sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_NUM_ITERS) {
			double measure_average = this->measure_sum_scores / (double)this->state_iter;
			cout << "measure_average: " << measure_average << endl;

			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				NegateNetwork* new_negate_network = new NegateNetwork(wrapper->solution->num_states + s_index);
				this->scope_context->start_negate_networks.push_back(new_negate_network);
			}

			wrapper->solution->num_states += NEW_STATE_NUM_ADD;
			// wrapper->solution->generic_action_network->add_states(wrapper->solution->num_states);
			// wrapper->solution->generic_obs_network->add_states(wrapper->solution->num_states);
			for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
				Scope* scope = wrapper->solution->scopes[s_index];
				scope->start_obs_network->add_states(wrapper->solution->num_states);
				for (int n_index = 0; n_index < (int)scope->start_init_networks.size(); n_index++) {
					scope->start_init_networks[n_index]->add_states(wrapper->solution->num_states);
				}
				// scope->start_score_network->add_states(wrapper->solution->num_states);
				// scope->end_score_network->add_states(wrapper->solution->num_states);
				for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
						it != scope->nodes.end(); it++) {
					switch (it->second->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)it->second;

							for (int n_index = 0; n_index < (int)noop_node->init_networks.size(); n_index++) {
								noop_node->init_networks[n_index]->add_states(wrapper->solution->num_states);
							}
							noop_node->score_network->add_states(wrapper->solution->num_states);
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->second;

							action_node->obs_network->add_states(wrapper->solution->num_states);
							for (int n_index = 0; n_index < (int)action_node->init_networks.size(); n_index++) {
								action_node->init_networks[n_index]->add_states(wrapper->solution->num_states);
							}
							action_node->score_network->add_states(wrapper->solution->num_states);
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)it->second;

							for (int n_index = 0; n_index < (int)scope_node->init_networks.size(); n_index++) {
								scope_node->init_networks[n_index]->add_states(wrapper->solution->num_states);
							}
							scope_node->score_network->add_states(wrapper->solution->num_states);
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)it->second;

							for (int n_index = 0; n_index < (int)branch_node->init_networks.size(); n_index++) {
								branch_node->init_networks[n_index]->add_states(wrapper->solution->num_states);
							}
							branch_node->original_network->add_states(wrapper->solution->num_states);
							branch_node->branch_network->add_states(wrapper->solution->num_states);
						}
						break;
					}
				}
			}

			for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
				add_dependency_helper(this->scope_context,
									  this->best_dependencies[d_index],
									  0,
									  this->measure_init_networks[d_index]);
			}

			add(this->measure_new_network,
				wrapper);

			/**
			 * - includes "delete this"
			 */
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
