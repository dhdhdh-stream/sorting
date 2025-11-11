#include "explore_experiment.h"

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
#else
const int EXPLORE_EXPERIMENT_EXPLORE_ITERS = 400;
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
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	history->num_instances = 0;
}

void ExploreExperiment::check_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

		if (history->num_instances == history->explore_index) {
			ExploreInstance* explore_instance = new ExploreInstance();
			explore_instance->experiment = this;
			history->explore_instance = explore_instance;

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
			explore_instance->exit_next_node = possible_exits[random_index];

			uniform_int_distribution<int> new_scope_distribution(0, 1);
			if (new_scope_distribution(generator) == 0) {
				explore_instance->new_scope = create_new_scope(this->node_context->parent);
			}
			if (explore_instance->new_scope != NULL) {
				explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
				explore_instance->actions.push_back(-1);
				explore_instance->scopes.push_back(explore_instance->new_scope);
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
						explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
						explore_instance->actions.push_back(-1);

						int child_index = possible_child_indexes[child_index_distribution(generator)];
						explore_instance->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
					} else {
						explore_instance->step_types.push_back(STEP_TYPE_ACTION);

						explore_instance->actions.push_back(-1);

						explore_instance->scopes.push_back(NULL);
					}
				}
			}

			for (int s_index = 0; s_index < (int)explore_instance->step_types.size(); s_index++) {
				if (explore_instance->step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = scope_context;

					explore_instance->new_nodes.push_back(new_action_node);
				} else {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = scope_context;
					new_scope_node->id = scope_context->node_counter + s_index;

					new_scope_node->scope = explore_instance->scopes[s_index];

					explore_instance->new_nodes.push_back(new_scope_node);
				}
			}

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

	if (experiment_state->step_index >= (int)history->explore_instance->step_types.size()) {
		wrapper->node_context.back() = history->explore_instance->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (history->explore_instance->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeNode* scope_node = (ScopeNode*)history->explore_instance->new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
			scope_history->node_histories[scope_node->id] = scope_node_history;

			ScopeHistory* inner_scope_history = new ScopeHistory(history->explore_instance->scopes[experiment_state->step_index]);
			scope_node_history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(history->explore_instance->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			inner_scope_history->pre_obs = wrapper->problem->get_observations();
		}
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	history->explore_instance->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.back()->post_obs = wrapper->problem->get_observations();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::backprop(double target_val,
								 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	history->explore_instance->scope_history = wrapper->scope_histories[0];

	for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
		ScopeHistory* scope_history = history->stack_trace[l_index];
		Scope* scope = scope_history->scope;

		if ((int)scope->explore_pre_obs.size() < MEASURE_ITERS) {
			scope->explore_pre_obs.push_back(scope_history->pre_obs);
			scope->explore_post_obs.push_back(scope_history->post_obs);
		} else {
			uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.size()-1);
			int index = distribution(generator);
			scope->explore_pre_obs[index] = scope_history->pre_obs;
			scope->explore_post_obs[index] = scope_history->post_obs;
		}
	}

	history->explore_instance->surprise = target_val - wrapper->existing_result;

	if ((wrapper->best_explore_instances.back() == NULL
			|| wrapper->best_explore_instances.back()->surprise < history->explore_instance->surprise)) {
		if (wrapper->best_explore_instances.back() != NULL) {
			delete wrapper->best_explore_instances.back();
		}
		wrapper->best_explore_instances.back() = history->explore_instance;

		int index = wrapper->best_explore_instances.size()-1;
		while (true) {
			if (wrapper->best_explore_instances[index-1] == NULL
					|| wrapper->best_explore_instances[index-1]->surprise < wrapper->best_explore_instances[index]->surprise) {
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
		delete history->explore_instance;
	}

	this->state_iter++;
	if (this->state_iter >= EXPLORE_EXPERIMENT_EXPLORE_ITERS) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
