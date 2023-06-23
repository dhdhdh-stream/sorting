#include "scope.h"

using namespace std;



void Scope::activate(vector<double>& flat_vals,
					 vector<double>& state_vals,
					 vector<StateDefinition*>& state_types,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 int& exit_node_id,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	// TODO: switch back to updating context on scope start?

	early_exit_depth = -1;

	if (run_helper.experiment_scope_id == this->id) {
		run_helper.is_recursive = true;
	}

	run_helper.curr_depth++;
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}

	if (this->is_loop) {

	} else {
		history->node_histories.push_back(vector<AbstractNodeHistory*>());

		int curr_node_id = 0;
		while (true) {
			if (curr_node_id == -1) {
				break;
			}

			handle_node_activate_helper();

			if (early_exit_depth != -1) {
				break;
			}
		}
	}

	run_helper.curr_depth--;
}

void Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										vector<double>& flat_vals,
										vector<double>& state_vals,
										vector<TypeDefinition*>& state_types,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										ScopeHistory* scope_history) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
		history->node_histories[iter_index].push_back(node_history);
		action_node->activate();

		if (action_node->is_explore
				&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			if (action_node->branch_experiment != NULL) {
				bool matches_context = true;
				if (action_node->branch_experiment->scope_context.size() > context.size()) {
					matches_context = false;
				} else {
					for (int c_index = 0; c_index < (int)action_node->branch_experiment->scope_context.size(); c_index++) {
						if (action_node->branch_experiment->scope_context[c_index] != 
									context[context.size()-action_node->branch_experiment->scope_context.size()+c_index].scope_id
								|| action_node->branch_experiment->node_context[c_index] !=
									context[context.size()-action_node->branch_experiment->scope_context.size()+c_index].node_id) {
							matches_context = false;
							break;
						}
					}
				}
				if (matches_context) {
					// explore_phase set in experiment
					BranchExperimentHistory* branch_experiment_history = new BranchExperimentHistory(action_node->branch_experiment);
					action_node->branch_experiment->activate(
						);

					if (early_exit_depth == 0) {
						curr_node_id = action_node->explore_exit_node_id;
					} else {
						early_exit_depth = action_node->explore_exit_depth;
						early_exit_node_id = action_node->explore_exit_node_id;
					}
				} else {
					curr_node_id = action_node->next_node_id;
				}
			} else if (action_node->loop_experiment != NULL) {

			} else {

			}
		}
	} else if () {

	}
}

void Scope::halfway_activate(vector<int>& starting_node_ids,
							 vector<vector<double>>& starting_state_vals,
							 vector<vector<StateDefinition*>>& starting_state_types,
							 vector<double>& flat_vals,
							 vector<double>& state_vals,
							 vector<StateDefinition*>& state_types,
							 vector<ContextLayer>& context,
							 int& exit_depth,
							 int& exit_node_id,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	early_exit_depth = -1;

	if (run_helper.experiment_scope_id == this->id) {
		run_helper.is_recursive = true;
	}

	run_helper.curr_depth++;
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}

	// can't be loop

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	starting_state_vals.erase(starting_state_vals.begin());
	starting_state_types.erase(starting_state_types.begin());
	if (starting_node_ids.size() != 0) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids,
									 starting_state_vals,
									 starting_state_types,
									 flat_vals,
									 state_vals,
									 state_types,
									 context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);


	}

	while (true) {

	}


}
