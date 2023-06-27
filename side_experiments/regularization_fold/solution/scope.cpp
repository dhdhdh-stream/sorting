#include "scope.h"

using namespace std;



void Scope::activate(vector<double>& flat_vals,
					 vector<ForwardContextLayer>& context,
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

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		map<int, vector<vector<StateNetwork*>>>::iterator state_it = run_helper.experiment->state_networks.find(this->id);
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (state_it == run_helper.experiment->state_networks.end()) {
			state_it = run_helper.experiment->state_networks.insert({this->id, vector<vector<StateNetwork*>>()}).first;
			score_it = run_helper.experiment->score_networks.insert({this->id, vector<ScoreNetwork*>()}).first;
		}

		run_helper.scope_state_networks = &(state_it->second);
		run_helper.scope_score_networks = &(score_it->second);

		int size_diff = (int)this->nodes.size() - run_helper.scope_state_networks->size();
		run_helper.scope_state_networks->insert(run_helper.scope_state_networks->end(), size_diff, vector<StateNetwork*>());
		run_helper.scope_score_networks->insert(run_helper.scope_score_networks->end(), size_diff, NULL);
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

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		run_helper.scope_state_networks = NULL;
		run_helper.scope_score_networks = NULL;

		if (run_helper.experiment_on_path) {
			run_helper.experiment_context_index--;
		}
	}

	run_helper.curr_depth--;
}

void Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										vector<double>& flat_vals,
										vector<ForwardContextLayer>& context,
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
							 vector<vector<double>*>& starting_state_vals,
							 vector<vector<bool>>& starting_states_initialized,
							 vector<double>& flat_vals,
							 vector<ForwardContextLayer>& context,
							 int& exit_depth,
							 int& exit_node_id,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	exit_depth = -1;

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
	if (starting_node_ids.size() > 0) {
		context.back().node_id = 0;

		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids,
									 starting_state_vals,
									 starting_states_initialized,
									 flat_vals,
									 context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);

		context.back().node_id = -1;

		if (inner_exit_depth == 0) {
			curr_node_id = inner_exit_node_id;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node_id = inner_exit_node_id;
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		map<int, vector<vector<StateNetwork*>>>::iterator state_it = run_helper.experiment->state_networks.find(this->id);
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (state_it == run_helper.experiment->state_networks.end()) {
			state_it = run_helper.experiment->state_networks.insert({this->id, vector<vector<StateNetwork*>>()}).first;
			score_it = run_helper.experiment->score_networks.insert({this->id, vector<ScoreNetwork*>()}).first;
		}

		run_helper.scope_state_networks = &(state_it->second);
		run_helper.scope_score_networks = &(score_it->second);

		int size_diff = (int)this->nodes.size() - run_helper.scope_state_networks->size();
		run_helper.scope_state_networks->insert(run_helper.scope_state_networks->end(), size_diff, vector<StateNetwork*>());
		run_helper.scope_score_networks->insert(run_helper.scope_score_networks->end(), size_diff, NULL);
	}

	while (true) {
		if (curr_node_id == -1 || exit_depth != -1) {
			break;
		}

		handle_node_activate_helper();
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		run_helper.scope_state_networks = NULL;
		run_helper.scope_score_networks = NULL;

		if (run_helper.experiment_on_path) {
			run_helper.experiment_context_index--;
		}
	}

	run_helper.curr_depth--;
}
