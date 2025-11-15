#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_instance.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_EXPERIMENT_EXPLORE_ITERS = 10;
const int EXPLORE_NUM_INSTANCES = 2;
#else
const int EXPLORE_EXPERIMENT_EXPLORE_ITERS = 400;
// const int EXPLORE_NUM_INSTANCES = 5;
const int EXPLORE_NUM_INSTANCES = 1;
#endif /* MDEBUG */

void ExploreExperiment::result_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		uniform_int_distribution<int> select_distribution(0, history->num_instances);
		if (select_distribution(generator) == 0) {
			history->explore_index = history->num_instances;
		}
		history->num_instances++;
	}
}

void ExploreExperiment::result_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->total_count++;

	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	history->num_instances = 0;
}

void ExploreExperiment::check_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

		if (history->num_instances == history->explore_index) {
			wrapper->has_explore = true;

			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
		history->num_instances++;
	}
}

void ExploreExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		history->stack_trace = wrapper->scope_histories;
	}

	if (experiment_state->step_index >= (int)this->curr_explore_instance->step_types.size()) {
		wrapper->node_context.back() = this->curr_explore_instance->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_explore_instance->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			if (this->curr_explore_instance->actions[experiment_state->step_index] == -1) {
				is_next = true;
				fetch_action = true;

				wrapper->num_actions++;
			} else {
				action = this->curr_explore_instance->actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_explore_instance->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_explore_instance->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			inner_scope_history->pre_obs = wrapper->problem->get_observations();
		}
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	this->curr_explore_instance->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.back()->post_obs = wrapper->problem->get_observations();

	wrapper->post_scope_histories.push_back(wrapper->scope_histories.back()->copy_signal());

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::backprop(double target_val,
								 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	if (this->curr_explore_instance->post_scope_histories.size() == 0) {
		for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
			ScopeHistory* scope_history = history->stack_trace[l_index];
			Scope* scope = scope_history->scope;

			if ((int)scope->explore_pre_obs.size() < MAX_SAMPLES) {
				scope->explore_pre_obs.push_back(scope_history->pre_obs);
				scope->explore_post_obs.push_back(scope_history->post_obs);
			} else {
				uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.size()-1);
				int index = distribution(generator);
				scope->explore_pre_obs[index] = scope_history->pre_obs;
				scope->explore_post_obs[index] = scope_history->post_obs;
			}
		}
	}

	this->curr_explore_instance->post_scope_histories.push_back(wrapper->post_scope_histories);
	wrapper->post_scope_histories.clear();
	wrapper->has_explore = false;

	double curr_surprise = target_val - wrapper->existing_result;
	if (curr_surprise > this->curr_explore_instance->best_surprise) {
		this->curr_explore_instance->best_surprise = curr_surprise;
	}

	if (this->curr_explore_instance->post_scope_histories.size() >= EXPLORE_NUM_INSTANCES) {
		this->explore_instances.push_back(this->curr_explore_instance);

		this->state_iter++;
		if (this->state_iter >= EXPLORE_EXPERIMENT_EXPLORE_ITERS) {
			this->average_hits_per_run = (EXPLORE_EXPERIMENT_EXPLORE_ITERS * EXPLORE_NUM_INSTANCES) / (double)this->total_count;

			for (int e_index = 0; e_index < (int)this->explore_instances.size(); e_index++) {
				this->explore_instances[e_index]->best_surprise *= this->average_hits_per_run;

				if ((wrapper->best_explore_instances.back() == NULL
						|| wrapper->best_explore_instances.back()->best_surprise < this->explore_instances[e_index]->best_surprise)) {
					if (wrapper->best_explore_instances.back() != NULL) {
						delete wrapper->best_explore_instances.back();
					}
					wrapper->best_explore_instances.back() = this->explore_instances[e_index];

					int index = wrapper->best_explore_instances.size()-1;
					while (true) {
						if (wrapper->best_explore_instances[index-1] == NULL
								|| wrapper->best_explore_instances[index-1]->best_surprise < wrapper->best_explore_instances[index]->best_surprise) {
							ExploreInstance* temp = wrapper->best_explore_instances[index];
							wrapper->best_explore_instances[index] = wrapper->best_explore_instances[index-1];
							wrapper->best_explore_instances[index-1] = temp;
						} else {
							break;
						}

						index--;
						if (index == 0) {
							break;
						}
					}
				} else {
					delete this->explore_instances[e_index];
				}
			}

			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			this->curr_explore_instance = new ExploreInstance();
			this->curr_explore_instance->experiment = this;

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
			this->curr_explore_instance->exit_next_node = possible_exits[random_index];

			uniform_int_distribution<int> new_scope_distribution(0, 1);
			if (new_scope_distribution(generator) == 0) {
				this->curr_explore_instance->new_scope = create_new_scope(this->node_context->parent);
			}
			if (this->curr_explore_instance->new_scope != NULL) {
				this->curr_explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_explore_instance->actions.push_back(-1);
				this->curr_explore_instance->scopes.push_back(this->curr_explore_instance->new_scope);
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
						this->curr_explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
						this->curr_explore_instance->actions.push_back(-1);

						int child_index = possible_child_indexes[child_index_distribution(generator)];
						this->curr_explore_instance->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
					} else {
						this->curr_explore_instance->step_types.push_back(STEP_TYPE_ACTION);

						this->curr_explore_instance->actions.push_back(-1);

						this->curr_explore_instance->scopes.push_back(NULL);
					}
				}
			}
		}
	}
}
