#include "explore.h"

#include "action_node.h"
#include "constants.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void Explore::experiment_check_activate(vector<double>& obs,
										SolutionWrapper* wrapper) {
	switch (wrapper->run_type) {
	case RUN_TYPE_EXPLORE:
		{
			map<Explore*, ExploreHistory*>::iterator it =
				wrapper->explore_histories.find(this);
			if (it == wrapper->explore_histories.end()) {
				it = wrapper->explore_histories.insert({this, new ExploreHistory(this)}).first;
			}

			this->num_instances_until_target--;
			if (!it->second->has_explore
					&& this->num_instances_until_target <= 0) {
				it->second->has_explore = true;

				ExploreState* new_experiment_state = new ExploreState(this);
				new_experiment_state->step_index = 0;
				wrapper->experiment_context.back() = new_experiment_state;
			}
		}

		break;
	}
}

void Explore::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  bool& fetch_action,
							  SolutionWrapper* wrapper) {
	ExploreState* experiment_state = (ExploreState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->run_num_actions++;
		} else {
			ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
				this->indexes[experiment_state->step_index]];
			generic_scope_node->experiment_step(obs,
												action,
												is_next,
												wrapper);
		}
	}
}

void Explore::set_action(int action,
						 SolutionWrapper* wrapper) {
	ExploreState* experiment_state = (ExploreState*)wrapper->experiment_context.back();

	this->indexes[experiment_state->step_index] = action;
}

void Explore::experiment_step_callback(vector<double>& obs,
									   SolutionWrapper* wrapper) {
	ExploreState* experiment_state = (ExploreState*)wrapper->experiment_context.back();

	int action = this->indexes[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];
	generic_action_node->experiment_step_callback(obs,
												  wrapper);

	experiment_state->step_index++;
}

void Explore::experiment_exit_step(vector<double>& obs,
								   SolutionWrapper* wrapper) {
	ExploreState* experiment_state = (ExploreState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[
		this->indexes[experiment_state->step_index]];
	generic_scope_node->experiment_exit_step(obs,
											 wrapper);

	experiment_state->step_index++;
}
