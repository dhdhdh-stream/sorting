#include "explore_experiment.h"

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "pass_through_network.h"
#include "predict_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"
#include "utilities.h"

using namespace std;

void ExploreExperiment::new_state_measure_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

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

		this->existing_network->activate(wrapper->states.back());

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (curr_dependencies_is_hit[d_index]) {
				this->init_networks[d_index]->init_activate(
					curr_dependencies_state[d_index],
					new_state,
					curr_dependencies_obs[d_index]);
			}
		}
		this->new_network->init_activate(wrapper->states.back(),
										 new_state);

		bool is_branch;
		if (this->new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		if (is_branch) {
			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::new_state_measure_step(vector<double>& obs,
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
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
				this->best_indexes[experiment_state->step_index]];
			generic_scope_node->experiment_step(obs,
												action,
												is_next,
												wrapper);
		}
	}
}

void ExploreExperiment::new_state_measure_callback(vector<double>& obs,
												   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	int action = this->best_indexes[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];
	generic_action_node->experiment_step_callback(obs,
												  wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::new_state_measure_exit_step(vector<double>& obs,
													SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
		this->best_indexes[experiment_state->step_index]];
	generic_scope_node->experiment_exit_step(obs,
											 wrapper);

	experiment_state->step_index++;
}

void ExploreExperiment::new_state_measure_backprop(double target_val,
												   ExploreExperimentHistory* history,
												   SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		this->sum_vals += target_val;

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_MEASURE_NUM_DATAPOINTS) {
			double new_val_average = this->sum_vals / this->state_iter;

			double local_improvement = new_val_average - this->existing_val_average;

			double average_hits_per_run;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					average_hits_per_run = noop_node->average_instances_per_run / noop_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					average_hits_per_run = action_node->average_instances_per_run / action_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					average_hits_per_run = scope_node->average_instances_per_run / scope_node->average_instances_per_hit;
				}
				break;
			default:
			// case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						average_hits_per_run = branch_node->branch_average_instances_per_run / branch_node->branch_average_instances_per_hit;
					} else {
						average_hits_per_run = branch_node->original_average_instances_per_run / branch_node->original_average_instances_per_hit;
					}
				}
				break;
			}
			double global_improvement = average_hits_per_run * local_improvement;

			bool is_success = false;
			if (local_improvement > 0.0) {
				if (this->scope_context->measure_new_state_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = this->scope_context->measure_new_state_last_scores.begin();
							it != this->scope_context->measure_new_state_last_scores.end(); it++) {
						if (global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->measure_new_state_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (this->scope_context->measure_new_state_last_scores.size() >= NUM_LAST_TRACK) {
						this->scope_context->measure_new_state_last_scores.pop_front();
					}
					this->scope_context->measure_new_state_last_scores.push_back(global_improvement);
				} else {
					this->scope_context->measure_new_state_last_scores.push_back(global_improvement);
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_success || rand()%3 != 0) {
			#else
			if (is_success) {
			#endif /* MDEBUG */
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
								action_node->predict_network->add_states(scope->num_states);
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)it->second;

								scope_node->in_network->add_front_states(scope->num_states);
								scope_node->out_network->add_back_states(scope->num_states);
								scope_node->predict_network->add_states(scope->num_states);
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
										  this->init_networks[d_index]);
				}
				this->init_networks.clear();

				this->existing_network->add_states(this->scope_context->num_states);

				add(true,
					wrapper);
			} else {
				delete this;
			}
		}
	}
}
