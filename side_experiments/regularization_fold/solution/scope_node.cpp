#include "scope_node.h"

using namespace std;



void ScopeNode::activate(vector<double>& flat_vals,
						 vector<ForwardContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	// update current context's node ID outside in Scope

	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_vals = context.back().state_vals;
	vector<bool>* curr_states_initialized = &(context.back().states_initialized);

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	vector<double> inner_state_vals(inner_scope->num_states, 0.0);
	context.back().states_initialized = vector<bool>(inner_scope->num_states, false);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_target_layers[i_index] == 0) {
			if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
				if (curr_states_initialized->at(this->input_indexes[i_index])) {
					double val = curr_state_vals->at(this->input_indexes[i_index]);
					if (this->input_transformations[i_index] != NULL) {
						val = this->input_transformations[i_index]->forward(val);
					}
					inner_state_vals[this->input_target_indexes[i_index]] = val;

					context.back().states_initialized[this->input_target_indexes[i_index]] = true;
				}
			} else {
				context.back().states_initialized[this->input_target_indexes[i_index]] = true;
			}
		}
	}
	context.back().state_vals = &inner_state_vals;

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	int experiment_on_path_save;
	vector<vector<StateNetwork*>>* scope_state_networks_save;
	vector<ScoreNetwork*>* scope_score_networks_save;
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		experiment_on_path_save = run_helper.experiment_on_path;
		run_helper.experiment_on_path = false;
		scope_state_networks_save = run_helper.scope_state_networks;
		run_helper.scope_state_networks = NULL;
		scope_score_networks_save = run_helper.scope_score_networks;
		run_helper.scope_score_networks = NULL;
	}

	if (this->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_START) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		vector<vector<double>> starting_state_vals(this->starting_node_ids.size()-1);
		vector<vector<bool>> starting_states_initialized(this->starting_node_ids.size()-1);

		Scope* curr_scope = solution->scopes[this->inner_scope_id];
		for (int l_index = 0; l_index < this->starting_node_ids.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			starting_state_vals[l_index] = vector<double>(next_scope->num_states, 0.0);
			starting_states_initialized[l_index] = vector<bool>(next_scope->num_states, false);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_target_layers[i_index] != 0) {
				if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
					if (curr_states_initialized->at(this->input_indexes[i_index])) {
						double val = curr_state_vals->at(this->input_indexes[i_index]);
						if (this->input_transformations[i_index] != NULL) {
							val = this->input_transformations[i_index]->forward(val);
						}
						starting_state_vals[this->input_target_layers[i_index]-1][this->input_target_indexes[i_index]] = val;

						starting_states_initialized[this->input_target_layers[i_index]-1][this->input_target_indexes[i_index]] = true;
					}
				} else {
					starting_states_initialized[this->input_target_layers[i_index]-1][this->input_target_indexes[i_index]] = true;
				}
			}
		}

		// currently, starting_node_ids.size() == starting_state_vals.size()+1

		vector<vector<double>*> inner_starting_state_vals(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_starting_state_vals[l_index] = &starting_state_vals[l_index];
		}

		inner_scope->halfway_activate(starting_node_ids_copy,
									  inner_starting_state_vals,
									  starting_states_initialized,
									  flat_vals,
									  context,
									  inner_exit_depth,
									  inner_exit_node_id,
									  run_helper,
									  inner_scope_history);
	} else {
		inner_scope->activate(flat_vals,
							  context,
							  inner_exit_depth,
							  inner_exit_node_id,
							  run_helper,
							  inner_scope_history);
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_target_layers[i_index] == 0) {
			if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
				if (curr_states_initialized->at(this->input_indexes[i_index])) {
					double val = inner_state_vals[this->input_target_indexes[i_index]];
					if (this->input_transformations[i_index] != NULL) {
						val = this->input_transformations[i_index]->backward(val);
					}
					curr_state_vals->at(this->input_indexes[i_index]) = val;
				}
			} else {
				ClassDefinition* inner_input_class = inner_scope->default_state_classes[this->input_target_indexes[i_index]];
				run_helper.last_seen_vals[inner_input_class] = inner_state_vals[this->input_target_indexes[i_index]];
			}
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_CLEAN) {
		run_helper.experiment_on_path = experiment_on_path_save;
		run_helper.scope_state_networks = scope_state_networks_save;
		run_helper.scope_score_networks = scope_score_networks_save;
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::activate(vector<double>& flat_vals,
						 vector<ForwardContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	// update current context's node ID outside in Scope

	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_vals = context.back().state_vals;
	vector<bool>* curr_states_initialized = &(context.back().states_initialized);

	
}

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<vector<double>*>& starting_state_vals,
								 vector<vector<bool>>& starting_states_initialized,
								 vector<double>& flat_vals,
								 vector<ForwardContextLayer>& context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_vals = context.back().state_vals;
	vector<bool>* curr_states_initialized = &(context.back().states_initialized);

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	starting_state_vals.erase(starting_state_vals.begin());
	context.back().states_initialized = starting_states_initialized[0];
	starting_states_initialized.erase(starting_states_initialized.begin());

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	// no need to update run_helper.experiment_on_path as must already be false

	inner_scope->halfway_activate(starting_node_ids,
								  starting_state_vals,
								  starting_states_initialized,
								  flat_vals,
								  context,
								  inner_exit_depth,
								  inner_exit_node_id,
								  run_helper,
								  inner_scope_history);

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		// this->input_target_layers[i_index] == 0
		if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
			if (curr_states_initialized->at(this->input_indexes[i_index])) {
				double val = context.back().state_vals->at(this->input_target_indexes[i_index]);
				if (this->input_transformations[i_index] != NULL) {
					val = this->input_transformations[i_index]->backward(val);
				}
				curr_state_vals->at(this->input_indexes[i_index]) = val;
			}
		} else {
			ClassDefinition* inner_input_class = inner_scope->default_state_classes[this->input_target_indexes[i_index]];
			run_helper.last_seen_vals[inner_input_class] = context.back().state_vals->at(this->input_target_indexes[i_index]);
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;

	/**
	 * - if explore context includes halfway activate, context needs to include HALFWAY_START
	 *   - then HALFWAY_START's inner_input_indexes/inner_input_target_indexes also needs to be updated even if its scope not updated
	 */
}

void ScopeNode::backprop(vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_errors = context.back().state_errors;

	context.push_back(BackwardContextLayer());

	vector<double> inner_state_errors(inner_scope->num_states, 0.0);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_target_layers[i_index] == 0) {
			if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
				double error = curr_state_errors->at(this->input_indexes[i_index]);
				if (this->input_transformations[i_index] != NULL) {
					error = this->input_transformations[i_index]->backprop_backward(error);
				}
				inner_state_errors[this->input_target_indexes[i_index]] = error;
			} else {
				// do nothing
			}
		}
	}
	context.back().state_errors = &inner_state_errors;

	double inner_scale_factor_error = 0.0;

	if (this->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_START) {
		vector<vector<double>> starting_state_errors(this->starting_node_ids.size()-1);

		Scope* curr_scope = solution->scopes[this->inner_scope_id];
		for (int l_index = 0; l_index < this->starting_node_ids.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			starting_state_errors[l_index] = vector<double>(next_scope->num_states, 0.0);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_target_layers[i_index] != 0) {
				if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
					double error = curr_state_errors->at(this->input_indexes[i_index]);
					if (this->input_transformations[i_index] != NULL) {
						error = this->input_transformations[i_index]->backprop_backward(error);
					}
					inner_state_errors[this->input_target_indexes[i_index]] = error;
				} else {
					// do nothing
				}
			}
		}

		vector<vector<double>*> inner_starting_state_errors(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_starting_state_errors[l_index] = &starting_state_errors[l_index];
		}

		inner_scope->halfway_backprop(inner_starting_state_errors,
									  context,
									  inner_scale_factor_error,
									  run_helper,
									  history->inner_scope_history);
	} else {
		inner_scope->backprop(context,
							  inner_scale_factor_error,
							  run_helper,
							  history->inner_scope_history);
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEANUP) {
		this->scope_scale_mod->backprop(inner_scale_factor_error, 0.0002);

		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_target_layers[i_index] == 0) {
			if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
				double error = inner_state_errors[this->input_target_indexes[i_index]];
				if (this->input_transformations[i_index] != NULL) {
					error = this->input_transformations[i_index]->backprop_forward(error);
				}
				curr_state_errors->at(this->input_indexes[i_index]) = error;
			} else {
				// do nothing
			}
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::halfway_backprop(vector<vector<double>*>& starting_state_errors,
								 vector<BackwardContextLayer>& context,
								 double& scale_factor_error,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_errors = context.back().state_errors;

	context.push_back(BackwardContextLayer());

	context.back().state_errors = starting_state_errors[0];
	starting_state_errors.erase(starting_state_errors.begin());

	double inner_scale_factor_error = 0.0;

	inner_scope->halfway_backprop(starting_state_errors,
								  context,
								  inner_scale_factor_error,
								  run_helper,
								  history->inner_scope_history);

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEANUP) {
		this->scope_scale_mod->backprop(inner_scale_factor_error, 0.0002);

		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_target_layers[i_index] == 0) {
			if (this->input_types[i_index] == INPUT_TYPE_EXISTING) {
				double error = inner_state_errors[this->input_target_indexes[i_index]];
				if (this->input_transformations[i_index] != NULL) {
					error = this->input_transformations[i_index]->backprop_forward(error);
				}
				curr_state_errors->at(this->input_indexes[i_index]) = error;
			} else {
				// do nothing
			}
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}


