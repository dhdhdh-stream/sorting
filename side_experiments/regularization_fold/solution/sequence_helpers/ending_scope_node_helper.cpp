#include "ending_scope_node_helper.h"

using namespace std;

EndingScopeNodeActivateHelper::EndingScopeNodeActivateHelper(ScopeNode* scope_node) {
	this->scope_node = scope_node;
}

void EndingScopeNodeActivateHelper::forward(vector<int>& next_starting_node_ids,
											vector<vector<double>*>& next_starting_state_vals,
											vector<vector<bool>>& next_starting_states_initialized,
											vector<ForwardContextLayer>& context,
											RunHelper& run_helper) {
	run_helper.scale_factor *= this->scope_node->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->scope_node->inner_scope_id];

	this->curr_state_vals = context.back().state_vals;
	this->curr_states_initialized = &(context.back().states_initialized);

	if (next_starting_node_ids.size() == 0) {
		this->is_halfway = false;

		this->inner_state_vals = vector<vector<double>>(this->scope_node->starting_node_ids.size());
		next_starting_states_initialized = vector<vector<bool>>(this->scope_node->starting_node_ids.size());
		this->inner_state_vals[0] = vector<double>(inner_scope->num_states, 0.0);
		next_starting_states_initialized[0] = vector<bool>(inner_scope->num_states, false);
		Scope* curr_scope = inner_scope;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			ScopeNode* l_scope_node = (ScopeNode*)curr_scope->nodes[this->scope_node->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[l_scope_node->inner_scope_id];

			this->inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
			next_starting_states_initialized[1+l_index] = vector<bool>(next_scope->num_states, false);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->curr_states_initialized->at(this->scope_node->input_indexes[i_index])) {
				double val = curr_state_vals->at(this->scope_node->input_indexes[i_index]);
				if (this->scope_node->input_has_transform[i_index]) {
					val = this->scope_node->input_transformations[i_index].forward(val);
				}
				this->inner_state_vals[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]] = val;

				next_starting_states_initialized[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]] = true;
			}
		}

		context.push_back(ForwardContextLayer());

		context.back().scope_id = -1;
		context.back().node_id = -1;

		context.back().state_vals = &(this->inner_state_vals[0]);
		context.back().states_initialized = next_starting_states_initialized[0];
		next_starting_states_initialized.erase(next_starting_states_initialized.begin());

		// no need to set context.back().scope_history

		next_starting_node_ids = this->scope_node->starting_node_ids;
		next_starting_node_ids.erase(next_starting_node_ids.begin());

		next_starting_state_vals = vector<vector<double>*>(this->scope_node->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			next_starting_state_vals[l_index] = &(this->inner_state_vals[1+l_index]);
		}
	} else {
		this->is_halfway = true;

		this->furthest_matching_layer = 0;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size(); l_index++) {
			if (l_index >= (int)next_starting_node_ids.size()
					|| next_starting_node_ids[l_index] != this->scope_node->starting_node_ids[l_index]) {
				break;
			} else {
				this->furthest_matching_layer++;
			}
		}
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->scope_node->input_target_layers[i_index] <= this->furthest_matching_layer) {
				double val = curr_state_vals->at(this->scope_node->input_indexes[i_index]);
				if (this->scope_node->input_has_transform[i_index]) {
					val = this->scope_node->input_transformations[i_index].forward(val);
				}
				next_starting_state_vals[this->scope_node->input_target_layers[i_index]]->at(this->scope_node->input_target_indexes[i_index]) = val;

				next_starting_states_initialized[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]] = true;
			}
		}

		context.push_back(ForwardContextLayer());

		context.back().scope_id = -1;
		context.back().node_id = -1;

		context.back().state_vals = next_starting_state_vals[0];
		context.back().states_initialized = next_starting_states_initialized[0];
		next_starting_states_initialized.erase(next_starting_states_initialized.begin());

		// no need to set context.back().scope_history

		next_starting_node_ids.erase(next_starting_node_ids.begin());

		this->starting_state_vals_save = next_starting_state_vals;
		next_starting_state_vals.erase(next_starting_state_vals.begin());
	}

	run_helper.experiment_helper_scope_context.push_back(this->scope_node->parent->id);
	run_helper.experiment_helper_node_context.push_back(this->scope_node->id);
	// only needed for EXPERIMENT_STATE_SECOND_CLEAN, but will simply update in general for now
}

void EndingScopeNodeActivateHelper::backward(vector<ForwardContextLayer>& context,
											 RunHelper& run_helper) {
	run_helper.experiment_helper_scope_context.pop_back();
	run_helper.experiment_helper_node_context.pop_back();

	if (this->is_halfway == false) {
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->curr_states_initialized->at(this->scope_node->input_indexes[i_index])) {
				double val = this->inner_state_vals[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]];
				if (this->scope_node->input_has_transform[i_index]) {
					val = this->scope_node->input_transformations[i_index].backward(val);
				}
				this->curr_state_vals->at(this->scope_node->input_indexes[i_index]) = val;
			}
		}
	} else {
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->scope_node->input_target_layers[i_index] <= this->furthest_matching_layer) {
				if (this->curr_states_initialized->at(this->scope_node->input_indexes[i_index])) {
					double val = this->starting_state_vals_save[this->scope_node->input_target_layers[i_index]]->at(this->scope_node->input_target_indexes[i_index]);
					if (this->scope_node->input_has_transform[i_index]) {
						val = this->scope_node->input_transformations[i_index].backward(val);
					}
					this->curr_state_vals->at(this->scope_node->input_indexes[i_index]) = val;
				}
			}
		}
	}

	context.pop_back();

	run_helper.scale_factor /= this->scope_node->scope_scale_mod->weight;
}

EndingScopeNodeBackpropHelper::EndingScopeNodeBackpropHelper(ScopeNode* scope_node) {
	this->scope_node = scope_node;
}

void EndingScopeNodeBackpropHelper::backward(vector<int>& next_starting_node_ids,
											 vector<vector<double>*>& next_starting_state_errors,
											 vector<BackwardContextLayer>& context,
											 RunHelper& run_helper) {
	run_helper.scale_factor *= this->scope_node->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->scope_node->inner_scope_id];

	this->curr_state_errors = context.back().state_errors;

	if (next_starting_node_ids.size() == 0) {
		this->is_halfway = false;

		this->inner_state_errors = vector<vector<double>>(this->scope_node->starting_node_ids.size());
		this->inner_state_errors[0] = vector<double>(inner_scope->num_states, 0.0);
		Scope* curr_scope = inner_scope;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			ScopeNode* l_scope_node = (ScopeNode*)curr_scope->nodes[this->scope_node->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[l_scope_node->inner_scope_id];

			this->inner_state_errors[1+l_index] = vector<double>(next_scope->num_states, 0.0);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			double error = curr_state_errors->at(this->scope_node->input_indexes[i_index]);
			if (this->scope_node->input_has_transform[i_index]) {
				error = this->scope_node->input_transformations[i_index].backprop_backward(error);
			}
			this->inner_state_errors[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]] = error;
		}

		context.push_back(BackwardContextLayer());

		context.back().state_errors = &(this->inner_state_errors[0]);

		next_starting_node_ids = this->starting_node_ids;
		next_starting_node_ids.erase(next_starting_node_ids.begin());

		next_starting_state_errors = vector<vector<double>*>(this->scope_node->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			next_starting_state_errors[l_index] = &(this->inner_state_errors[1+l_index]);
		}
	} else {
		this->is_halfway = true;

		this->furthest_matching_layer = 0;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size(); l_index++) {
			if (l_index >= (int)next_starting_node_ids.size()
					|| next_starting_node_ids[l_index] != this->scope_node->starting_node_ids[l_index]) {
				break;
			} else {
				this->furthest_matching_layer++;
			}
		}
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->scope_node->input_target_layers[i_index] <= this->furthest_matching_layer) {
				double error = curr_state_errors->at(this->scope_node->input_indexes[i_index]);
				if (this->scope_node->input_has_transform[i_index]) {
					error = this->scope_node->input_transformations[i_index].backprop_backward(error);
				}
				next_starting_state_errors[this->scope_node->input_target_layers[i_index]]->at(this->scope_node->input_target_indexes[i_index]) = error;
			}
		}

		context.push_back(BackwardContextLayer());

		context.back().state_errors = next_starting_state_errors[0];

		next_starting_node_ids.erase(next_starting_node_ids.begin());

		this->starting_state_errors_save = next_starting_state_errors;
		next_starting_state_errors.erase(next_starting_state_errors.begin());
	}
}

void EndingScopeNodeBackpropHelper::forward(vector<BackwardContextLayer>& context,
											double& cumulative_scale_factor_error,
											RunHelper& run_helper) {
	if (this->is_halfway == false) {
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			double error = this->inner_state_errors[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[i_index]];
			if (this->scope_node->input_has_transform[i_index]) {
				error = this->scope_node->input_transformations[i_index].backprop_forward(error);
			}
			this->curr_state_errors->at(this->scope_node->input_indexes[i_index]) = error;
		}
	} else {
		for (int i_index = 0; i_index < (int)this->scope_node->input_indexes.size(); i_index++) {
			if (this->scope_node->input_target_layers[i_index] <= this->furthest_matching_layer) {
				double error = this->starting_state_errors_save[this->scope_node->input_target_layers[i_index]]->at(this->scope_node->input_target_indexes[i_index]);
				if (this->scope_node->input_has_transform[i_index]) {
					error = this->scope_node->input_transformations[i_index].backprop_forward(error);
				}
				this->curr_state_errors->at(this->scope_node->input_indexes[i_index]) = error;
			}
		}
	}

	cumulative_scale_factor_error *= this->scope_node->scope_scale_mod->weight;

	context.pop_back();

	run_helper.scale_factor /= this->scope_node->scope_scale_mod->weight;
}
