#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"

using namespace std;

ScopeNode::ScopeNode(Scope* parent,
					 int id,
					 int inner_scope_id,
					 vector<int> starting_node_ids,
					 vector<int> input_indexes,
					 vector<int> input_target_layers,
					 vector<int> input_target_indexes,
					 vector<bool> input_has_transform,
					 vector<Transformation> input_transformations,
					 Scale* scope_scale_mod,
					 int next_node_id) {
	this->type = NODE_TYPE_SCOPE;

	this->parent = parent;
	this->id = id;

	this->inner_scope_id = inner_scope_id;
	this->starting_node_ids = starting_node_ids;
	this->input_indexes = input_indexes;
	this->input_target_layers = input_target_layers;
	this->input_target_indexes = input_target_indexes;
	this->input_has_transform = input_has_transform;
	this->input_transformations = input_transformations;
	this->scope_scale_mod = scope_scale_mod;
	this->next_node_id = next_node_id;
}

ScopeNode::ScopeNode(ScopeNode* original,
					 Scope* parent,
					 int id,
					 int next_node_id) {
	this->type = NODE_TYPE_SCOPE;

	this->parent = parent;
	this->id = id;

	this->inner_scope_id = original->inner_scope_id;
	this->starting_node_ids = original->starting_node_ids;
	this->input_indexes = original->input_indexes;
	this->input_target_layers = original->input_target_layers;
	this->input_target_indexes = original->input_target_indexes;
	this->input_has_transform = original->input_has_transform;
	this->input_transformations = original->input_transformations;
	this->scope_scale_mod = original->scope_scale_mod;
	this->next_node_id = next_node_id;
}

ScopeNode::ScopeNode(ifstream& input_file,
					 Scope* parent,
					 int id) {
	this->type = NODE_TYPE_SCOPE;

	this->parent = parent;
	this->id = id;

	string inner_scope_id_line;
	getline(input_file, inner_scope_id_line);
	this->inner_scope_id = stoi(inner_scope_id_line);

	string starting_node_ids_size_line;
	getline(input_file, starting_node_ids_size_line);
	int starting_node_ids_size = stoi(starting_node_ids_size_line);
	for (int l_index = 0; l_index < starting_node_ids_size; l_index++) {
		string starting_node_id_line;
		getline(input_file, starting_node_id_line);
		this->starting_node_ids.push_back(stoi(starting_node_id_line));
	}

	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	for (int i_index = 0; i_index < input_size; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->input_indexes.push_back(stoi(input_index_line));

		string input_target_layer_line;
		getline(input_file, input_target_layer_line);
		this->input_target_layers.push_back(stoi(input_target_layer_line));

		string input_target_index_line;
		getline(input_file, input_target_index_line);
		this->input_target_indexes.push_back(stoi(input_target_index_line));

		string input_has_transform_line;
		getline(input_file, input_has_transform_line);
		this->input_has_transform.push_back(stoi(input_has_transform_line));

		this->input_transformations.push_back(Transformation(input_file));
	}

	string scope_scale_mod_weight_line;
	getline(input_file, scope_scale_mod_weight_line);
	this->scope_scale_mod = new Scale(stod(scope_scale_mod_weight_line));

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

ScopeNode::~ScopeNode() {
	delete this->scope_scale_mod;
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
		if (context.back().states_initialized[this->input_indexes[i_index]]) {
			double val = context.back().state_vals->at(this->input_indexes[i_index]);
			if (this->input_has_transform[i_index]) {
				val = this->input_transformations[i_index].forward(val);
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

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	if (run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		run_helper.experiment_helper_scope_context.push_back(this->parent->id);
		run_helper.experiment_helper_node_context.push_back(this->id);
	}

	inner_scope->activate(starting_node_ids_copy,
						  inner_state_vals_copy,
						  inner_states_initialized,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	if (run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		run_helper.experiment_helper_scope_context.pop_back();
		run_helper.experiment_helper_node_context.pop_back();
	}

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (context.back().states_initialized[this->input_indexes[i_index]]) {
			double val = inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			if (this->input_has_transform[i_index]) {
				val = this->input_transformations[i_index].backward(val);
			}
			context.back().state_vals->at(this->input_indexes[i_index]) = val;
		}
	}

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
			if (context.back().states_initialized[this->input_indexes[i_index]]) {
				double val = context.back().state_vals->at(this->input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
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

	if (run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		run_helper.experiment_helper_scope_context.push_back(this->parent->id);
		run_helper.experiment_helper_node_context.push_back(this->id);
	}

	inner_scope->activate(starting_node_ids,
						  inner_state_vals,
						  starting_states_initialized,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	if (run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		run_helper.experiment_helper_scope_context.pop_back();
		run_helper.experiment_helper_node_context.pop_back();
	}

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (context.back().states_initialized[this->input_indexes[i_index]]) {
				double val = starting_state_vals[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].backward(val);
				}
				context.back().state_vals->at(this->input_indexes[i_index]) = val;
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::backprop(vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		double inner_scale_factor_error = 0.0;

		vector<int> empty_starting_node_ids_copy;
		vector<vector<double>*> empty_inner_state_errors_copy;
		inner_scope->backprop(empty_starting_node_ids_copy,
							  empty_inner_state_errors_copy,
							  context,
							  inner_scale_factor_error,
							  run_helper,
							  history->inner_scope_history);

		this->scope_scale_mod->backprop(inner_scale_factor_error, 0.0002);

		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (run_helper.backprop_is_pre_experiment) {
			// if pre-experiment, context no longer needed

			// unused
			vector<int> starting_node_ids_copy;
			vector<vector<double>*> inner_state_errors_copy;
			double inner_scale_factor_error = 0.0;

			inner_scope->backprop(starting_node_ids_copy,
								  inner_state_errors_copy,
								  context,
								  inner_scale_factor_error,
								  run_helper,
								  history->inner_scope_history);
		} else {
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
				double error = context.back().state_errors->at(this->input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					error = this->input_transformations[i_index].backprop_backward(error);
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

			scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;

			context.pop_back();

			for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
				double error = inner_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
				if (this->input_has_transform[i_index]) {
					error = this->input_transformations[i_index].backprop_forward(error);
				}
				context.back().state_errors->at(this->input_indexes[i_index]) = error;
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::halfway_backprop(vector<int>& starting_node_ids,
								 vector<vector<double>*>& starting_state_errors,
								 vector<BackwardContextLayer>& context,
								 double& scale_factor_error,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
	// (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT || run_helper.explore_phase == EXPLORE_PHASE_CLEAN) && !run_helper.backprop_is_pre_experiment

	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

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
			double error = context.back().state_errors->at(this->input_indexes[i_index]);
			if (this->input_has_transform[i_index]) {
				error = this->input_transformations[i_index].backprop_backward(error);
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

	scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			double error = starting_state_errors[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]);
			if (this->input_has_transform[i_index]) {
				error = this->input_transformations[i_index].backprop_forward(error);
			}
			context.back().state_errors->at(this->input_indexes[i_index]) = error;
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->inner_scope_id << endl;

	output_file << this->starting_node_ids.size() << endl;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		output_file << this->starting_node_ids[l_index] << endl;
	}

	output_file << this->input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		output_file << this->input_indexes[i_index] << endl;
		output_file << this->input_target_layers[i_index] << endl;
		output_file << this->input_target_indexes[i_index] << endl;
		output_file << this->input_has_transform[i_index] << endl;
		this->input_transformations[i_index].save(output_file);
	}

	output_file << this->scope_scale_mod->weight << endl;

	output_file << this->next_node_id << endl;
}

void ScopeNode::save_for_display(ofstream& output_file) {

}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;

	this->inner_scope_history = new ScopeHistory(original->inner_scope_history);
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->inner_scope_history;
}
