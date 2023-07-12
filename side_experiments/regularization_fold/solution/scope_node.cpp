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

	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<bool>> inner_states_initialized(this->starting_node_ids.size());
	inner_state_vals[0] = vector<double>(inner_scope->num_states, 0.0);
	inner_states_initialized[0] = vector<bool>(inner_scope->num_states, false);
	Scope* curr_scope = inner_scope;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		inner_states_initialized[1+l_index] = vector<bool>(next_scope->num_states, false);

		curr_scope = next_scope;
	}
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (curr_states_initialized->at(this->input_indexes[i_index])) {
			double val = curr_state_vals->at(this->input_indexes[i_index]);
			if (this->input_transformations[i_index] != NULL) {
				val = this->input_transformations[i_index]->forward(val);
			}
			inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = val;

			inner_states_initialized[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = true;
		}
	}

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);
	context.back().states_initialized = inner_states_initialized[0];
	inner_states_initialized.erase(inner_states_initialized.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
	}

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	bool experiment_on_path_save = run_helper.experiment_on_path;
	run_helper.experiment_on_path = false;
	vector<vector<StateNetwork*>>* scope_state_networks_save = run_helper.scope_state_networks;
	run_helper.scope_state_networks = NULL;
	vector<ScoreNetwork*>* scope_score_networks_save = run_helper.scope_score_networks;
	run_helper.scope_score_networks = NULL;

	inner_scope->activate(starting_node_ids_copy,
						  inner_state_vals_copy,
						  inner_states_initialized,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	run_helper.experiment_on_path = experiment_on_path_save;
	run_helper.scope_state_networks = scope_state_networks_save;
	run_helper.scope_score_networks = scope_score_networks_save;

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (curr_states_initialized->at(this->input_indexes[i_index])) {
			double val = inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			if (this->input_transformations[i_index] != NULL) {
				val = this->input_transformations[i_index]->backward(val);
			}
			curr_state_vals->at(this->input_indexes[i_index]) = val;
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
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

	int furthest_matching_layer = 0;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		if (l_index >= (int)starting_node_ids.size()
				|| starting_node_ids[l_index] != this->starting_node_ids[l_index]) {
			break;
		} else {
			furthest_matching_layer++;
		}
	}
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (curr_states_initialized->at(this->input_indexes[i_index])) {
				double val = curr_state_vals->at(this->input_indexes[i_index]);
				if (this->input_transformations[i_index] != NULL) {
					val = this->input_transformations[i_index]->forward(val);
				}
				starting_state_vals[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]) = val;

				starting_states_initialized[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = true;
			}
		}
	}

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	context.back().states_initialized = starting_states_initialized[0];
	starting_states_initialized.erase(starting_states_initialized.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<vector<double>*> inner_state_vals(starting_state_vals.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_vals.size()-1; l_index++) {
		inner_state_vals[l_index] = starting_state_vals[1+l_index];
	}

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	bool experiment_on_path_save = run_helper.experiment_on_path;
	run_helper.experiment_on_path = false;
	vector<vector<StateNetwork*>>* scope_state_networks_save = run_helper.scope_state_networks;
	run_helper.scope_state_networks = NULL;
	vector<ScoreNetwork*>* scope_score_networks_save = run_helper.scope_score_networks;
	run_helper.scope_score_networks = NULL;

	inner_scope->activate(starting_node_ids,
						  inner_state_vals,
						  starting_states_initialized,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	run_helper.experiment_on_path = experiment_on_path_save;
	run_helper.scope_state_networks = scope_state_networks_save;
	run_helper.scope_score_networks = scope_score_networks_save;

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (curr_states_initialized->at(this->input_indexes[i_index])) {
				double val = starting_state_vals[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]);
				if (this->input_transformations[i_index] != NULL) {
					val = this->input_transformations[i_index]->backward(val);
				}
				curr_state_vals->at(this->input_indexes[i_index]) = val;
			}
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::backprop(vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_errors = context.back().state_errors;

	vector<vector<double>> inner_state_errors(this->starting_node_ids.size());
	inner_state_errors[0] = vector<double>(inner_scope->num_states, 0.0);
	Scope* curr_scope = inner_scope;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_errors[1+l_index] = vector<double>(next_scope->num_states, 0.0);

		curr_scope = next_scope;
	}
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		double error = curr_state_errors->at(this->input_indexes[i_index]);
		if (this->input_transformations[i_index] != NULL) {
			error = this->input_transformations[i_index]->backprop_backward(error);
		}
		inner_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = error;
	}

	context.push_back(BackwardContextLayer());

	context.back().state_errors = &(inner_state_errors[0]);

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_errors_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_errors_copy[l_index] = &(inner_state_errors[1+l_index]);
	}

	double inner_scale_factor_error = 0.0;

	inner_scope->backprop(starting_node_ids_copy,
						  inner_state_errors_copy,
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

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		double error = inner_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
		if (this->input_transformations[i_index] != NULL) {
			error = this->input_transformations[i_index]->backprop_forward(error);
		}
		curr_state_errors->at(this->input_indexes[i_index]) = error;
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::halfway_backprop(vector<int>& starting_node_ids,
								 vector<vector<double>*>& starting_state_errors,
								 vector<BackwardContextLayer>& context,
								 double& scale_factor_error,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<double>* curr_state_errors = context.back().state_errors;

	int furthest_matching_layer = 0;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		if (l_index >= (int)starting_node_ids.size()
				|| starting_node_ids[l_index] != this->starting_node_ids[l_index]) {
			break;
		} else {
			furthest_matching_layer++;
		}
	}
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			double error = curr_state_errors->at(this->input_indexes[i_index]);
			if (this->input_transformations[i_index] != NULL) {
				error = this->input_transformations[i_index]->backprop_backward(error);
			}
			starting_state_errors[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]) = error;
		}
	}

	context.push_back(BackwardContextLayer());

	context.back().state_errors = starting_state_errors[0];

	vector<vector<double>*> inner_state_errors(starting_state_errors.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_errors.size()-1; l_index++) {
		inner_state_errors[l_index] = starting_state_errors[1+l_index];
	}

	double inner_scale_factor_error = 0.0;

	inner_scope->backprop(starting_node_ids,
						  inner_state_errors,
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

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			double error = starting_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			if (this->input_transformations[i_index] != NULL) {
				error = this->input_transformations[i_index]->backprop_forward(error);
			}
			curr_state_errors->at(this->input_indexes[i_index]) = error;
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}


