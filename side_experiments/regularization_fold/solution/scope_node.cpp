#include "scope_node.h"

using namespace std;



void ScopeNode::activate(vector<double>& flat_vals,
						 vector<double>& state_vals,
						 vector<StateDefinition*>& state_types,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 int& exit_node_id,
						 ScopeHistory* scope_history,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	context.push_back(ContextLayer(this->parent->id,
								   this->id,
								   &state_vals,
								   &state_types,
								   scope_history));

	scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> input_vals(inner_scope->num_states, 0.0);
	vector<StateDefinition*> input_types(inner_scope->num_states, NULL);
	for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
		double val = state_vals[this->inner_input_indexes[i_index]];
		if (this->inner_input_transformations[i_index] != NULL) {
			val = this->inner_input_transformations[i_index]->forward(val);
		}
		input_vals[this->inner_input_target_indexes[i_index]] = val;

		if (this->inner_input_types[i_index] == NULL) {
			input_types[i_index] = state_types[this->inner_input_indexes[i_index]];
		} else {
			input_types[i_index] = this->inner_input_types[i_index];
		}
	}

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	inner_scope->activate(flat_vals,
						  input_vals,
						  input_types,
						  context,
						  exit_depth,
						  exit_node_id,
						  run_helper,
						  inner_scope_history);

	for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
		double val = input_vals[this->inner_input_target_indexes[i_index]];
		if (this->inner_input_transformations[i_index] != NULL) {
			val = this->inner_input_transformations[i_index]->backward(val);
		}
		state_vals[this->inner_input_indexes[i_index]] = val;
	}

	scale_factor /= this->scope_scale_mod->weight;

	context.pop_back();
}
