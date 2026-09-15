#include "explore_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void ExploreExperiment::measure_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	bool is_branch;
	this->existing_network->activate(obs);
	double existing_predicted = this->existing_network->output->acti_vals[0];
	this->new_network->activate(obs);
	double new_predicted = this->new_network->output->acti_vals[0];
	if (new_predicted >= existing_predicted) {
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
	this->sum_vals += target_val;

	wrapper->new_since_update++;

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		double new_val_average = this->sum_vals / this->state_iter;

		#if defined(MDEBUG) && MDEBUG
		if (new_val_average > this->existing_val_average || rand()%3 != 0) {
		#else
		if (new_val_average > this->existing_val_average) {
		#endif /* MDEBUG */
			add(wrapper);

			delete this;

			wrapper->experiment_iter++;
			if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
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
									noop_node->experiment = NULL;
								}
							}
							break;
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)it->second;
								if (action_node->experiment != NULL) {
									delete action_node->experiment;
									action_node->experiment = NULL;
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)it->second;
								if (scope_node->experiment != NULL) {
									delete scope_node->experiment;
									scope_node->experiment = NULL;
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)it->second;
								if (branch_node->original_experiment != NULL) {
									delete branch_node->original_experiment;
									branch_node->original_experiment = NULL;
								}
								if (branch_node->branch_experiment != NULL) {
									delete branch_node->branch_experiment;
									branch_node->branch_experiment = NULL;
								}
							}
							break;
						}
					}
				}

				wrapper->experiment_iter = 0;
			}
		} else {
			this->try_iter++;
			if (this->try_iter >= EXPERIMENT_MAX_TRIES) {
				delete this;

				wrapper->experiment_iter++;
				if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
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
										noop_node->experiment = NULL;
									}
								}
								break;
							case NODE_TYPE_ACTION:
								{
									ActionNode* action_node = (ActionNode*)it->second;
									if (action_node->experiment != NULL) {
										delete action_node->experiment;
										action_node->experiment = NULL;
									}
								}
								break;
							case NODE_TYPE_SCOPE:
								{
									ScopeNode* scope_node = (ScopeNode*)it->second;
									if (scope_node->experiment != NULL) {
										delete scope_node->experiment;
										scope_node->experiment = NULL;
									}
								}
								break;
							case NODE_TYPE_BRANCH:
								{
									BranchNode* branch_node = (BranchNode*)it->second;
									if (branch_node->original_experiment != NULL) {
										delete branch_node->original_experiment;
										branch_node->original_experiment = NULL;
									}
									if (branch_node->branch_experiment != NULL) {
										delete branch_node->branch_experiment;
										branch_node->branch_experiment = NULL;
									}
								}
								break;
							}
						}
					}

					wrapper->experiment_iter = 0;
				}
			} else {
				this->new_obs_histories.clear();
				this->new_target_val_histories.clear();

				double average_instances_per_hit;
				switch (this->node_context->type) {
				case NODE_TYPE_NOOP:
					{
						NoopNode* noop_node = (NoopNode*)this->node_context;
						average_instances_per_hit = noop_node->average_instances_per_hit;
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)this->node_context;
						average_instances_per_hit = action_node->average_instances_per_hit;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)this->node_context;
						average_instances_per_hit = scope_node->average_instances_per_hit;
					}
					break;
				default:
				// case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)this->node_context;
						if (this->is_branch) {
							average_instances_per_hit = branch_node->branch_average_instances_per_hit;
						} else {
							average_instances_per_hit = branch_node->original_average_instances_per_hit;
						}
					}
					break;
				}
				uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
				this->num_instances_until_target = until_distribution(generator);

				this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
				this->state_iter = 0;
			}
		}
	}
}
