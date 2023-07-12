#include "scope.h"

using namespace std;



void Scope::activate(vector<int>& starting_node_ids,
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

	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	vector<bool> local_states_initialized(this->num_states, false);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		if (this->states_initialized[s_index]) {
			if (!context.back().states_initialized[s_index]) {
				local_states_initialized[s_index] = true;
				context.back().states_initialized[s_index] = true;
			}
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
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

		map<int, int>::iterator layer_seen_in_it = run_helper.experiment->scope_furthest_layer_seen_in.find(this->id);
		if (layer_seen_in_it == run_helper.experiment->scope_furthest_layer_seen_in.end()) {
			run_helper.experiment->scope_furthest_layer_seen_in[this->id] = run_helper.experiment_context_index;
		} else {
			if (run_helper.experiment_context_index < layer_seen_in_it->second) {
				layer_seen_in_it->second = run_helper.experiment_context_index;
			}
		}

		if (run_helper.experiment_step_index != -1) {
			BranchExperiment* branch_experiment = (BranchExperiment*)run_helper.experiment;
			map<int, vector<bool>>::iterator steps_seen_in_it = branch_experiment->scope_steps_seen_in.find(this->id);
			if (steps_seen_in_it == branch_experiment->scope_steps_seen_in.end()) {
				steps_seen_in_it = branch_experiment->scope_steps_seen_in.insert({this->id, vector<bool>(branch_experiment->num_steps, false)}).first;
			}
			steps_seen_in_it[run_helper.experiment_step_index] = true;
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_NEW_CLASSES
			|| run_helper.explore_phase == EXPLORE_PHASE_MEASURE) {
		map<int, vector<vector<StateNetwork*>>>::iterator state_it = run_helper.experiment->state_networks.find(this->id);
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (state_it == run_helper.experiment->state_networks.end()) {
			run_helper.scope_state_networks = NULL;
			run_helper.scope_score_networks = NULL;
		} else {
			run_helper.scope_state_networks = &(state_it->second);
			run_helper.scope_score_networks = &(score_it->second);
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_CLEANUP) {
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (score_it == run_helper.experiment->score_networks.end()) {
			run_helper.scope_score_networks = NULL;
		} else {
			run_helper.scope_score_networks = &(score_it->second);
		}
	}

	if (this->is_loop) {

	} else {
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

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;
			} else if (inner_exit_depth == 0) {
				curr_node_id = inner_exit_node_id;
			} else {
				exit_depth = inner_exit_depth-1;
				exit_node_id = inner_exit_node_id;
			}
		}

		while (true) {
			if (curr_node_id == -1 || exit_depth != -1) {
				break;
			}

			handle_node_activate_helper(0,
										curr_node_id,
										flat_vals,
										context,
										exit_depth,
										exit_node_id,
										run_helper,
										history);
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
		run_helper.scope_state_networks = NULL;
		run_helper.scope_score_networks = NULL;

		if (run_helper.experiment_on_path) {
			run_helper.experiment_context_index--;
		}
	}

	for (int s_index = 0; s_index < this->num_states; s_index++) {
		if (local_states_initialized[s_index]) {
			run_helper.last_seen_vals[this->default_state_classes[s_index]] = context.back().state_vals->at(s_index);
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
										ScopeHistory* history) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
		history->node_histories[iter_index].push_back(node_history);
		action_node->activate(flat_vals,
							  context,
							  run_helper,
							  node_history);

		if (action_node->is_explore
				&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			if (action_node->curr_experiment != NULL) {
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
					if (action_node->curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)action_node->curr_experiment;

						// explore_phase set in experiment
						BranchExperimentHistory* branch_experiment_history = new BranchExperimentHistory(branch_experiment);
						branch_experiment->activate(flat_vals,
													context,
													run_helper,
													branch_experiment_history);

						if (run_helper.explore_phase == EXPLORE_PHASE_CLEANUP
								&& branch_experiment_history->is_original) {
							curr_node_id = action_node->next_node_id;
						} else {
							if (branch_experiment->exit_depth == 0) {
								curr_node_id = branch_experiment->exit_node_id;
							} else {
								exit_depth = branch_experiment->exit_depth;
								exit_node_id = branch_experiment->exit_node_id;
							}
						}
					} else {
						// action_node->curr_experiment->type == EXPERIMENT_TYPE_LOOP

					}
				} else {
					curr_node_id = action_node->next_node_id;
				}
			} else {
				// TODO: explore

			}
		} else {
			curr_node_id = action_node->next_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[iter_index].push_back(node_history);
		scope_node->activate(flat_vals,
							 context,
							 inner_exit_depth,
							 inner_exit_node_id,
							 run_helper,
							 node_history);

		if (inner_exit_depth == -1) {
			curr_node_id = scope_node->next_node_id;
		} else if (inner_exit_depth == 0) {
			curr_node_id = inner_exit_node_id;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node_id = inner_exit_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
		history->node_histories[iter_index].push_back(node_history);
		branch_node->activate(context,
							  run_helper,
							  history);

		if (node_history->is_branch) {
			curr_node_id = branch_node->branch_next_node_id;
		} else {
			curr_node_id = branch_node->original_next_node_id;
		}
	} else {
		// this->nodes[curr_node_id]->type == NODE_TYPE_EXIT
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		ExitNodeHistory* node_history = new ExitNodeHistory(exit_node);
		history->node_histories[iter_index].push_back(node_history);
		exit_node->activate(context,
							run_helper,
							node_history);

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth;
			exit_node_id = exit_node->exit_node_id;
		}
	}
}

void Scope::backprop(vector<int>& starting_node_ids,
					 vector<vector<double>*>& starting_state_errors,
					 vector<BackwardContextLayer>& context,
					 double& scale_factor_error,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	if (is_loop) {

	} else {
		starting_node_ids.erase(starting_node_ids.begin());

		for (int h_index = (int)history->node_histories[0].size()-1; h_index >= 1; h_index--) {
			handle_node_backprop_helper(0,
										h_index,
										context,
										scale_factor_error,
										run_helper,
										history);
		}

		if (starting_state_errors.size() > 0) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[0][0]->node;
			scope_node->halfway_backprop(starting_node_ids,
										 starting_state_errors,
										 context,
										 scale_factor_error,
										 run_helper,
										 history);
		} else {
			handle_node_backprop_helper(0,
										0,
										context,
										scale_factor_error,
										run_helper,
										history);
		}
	}
}

void Scope::handle_node_backprop_helper(int iter_index,
										int h_index,
										vector<BackwardContextLayer>& context,
										double& scale_factor_error,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_ACTION) {
		ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[iter_index][h_index];
		ActionNode* action_node = (ActionNode*)action_node_history->node;
		action_node->backprop(context,
							  scale_factor_error,
							  run_helper,
							  action_node_history);

		if (action_node_history->experiment_history != NULL) {
			action_node->curr_experiment->backprop();

			if (action_node->curr_experiment->state == EXPERIMENT_STATE_DONE) {

			}
		}
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_SCOPE) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[iter_index][h_index];
		ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
		scope_node->backprop(context,
							 scale_factor_error,
							 run_helper,
							 scope_node_history);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_BRANCH) {
		BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history->node_histories[iter_index][h_index];
		BranchNode* branch_node = (BranchNode*)branch_node_history->node;
		branch_node->backprop(context,
							  run_helper,
							  branch_node_history);
	} else {
		// history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_EXIT
		ExitNodeHistory* exit_node_history = (ExitNodeHistory*)history->node_histories[iter_index][h_index];
		ExitNode* exit_node = (ExitNode*)exit_node_history->node;
		exit_node->backprop(context,
							run_helper,
							branch_node_history);
	}
}


