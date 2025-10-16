#include "refine_experiment.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "scope.h"

using namespace std;

RefineExperiment::RefineExperiment() {
	this->type = EXPERIMENT_TYPE_REFINE;

	this->existing_network = NULL;
	this->new_network = NULL;
	this->new_scope = NULL;

	this->num_refines = 0;
	this->state_iter = 0;
}

RefineExperiment::~RefineExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

void RefineExperiment::clean_inputs(Scope* scope,
									int node_id) {
	for (int i_index = (int)this->existing_network_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_network_inputs[i_index].scope_context.size(); l_index++) {
			if (this->existing_network_inputs[i_index].scope_context[l_index] == scope
					&& this->existing_network_inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->existing_network_inputs.erase(this->existing_network_inputs.begin() + i_index);
			this->existing_network->remove_input(i_index);

			for (int h_index = 0; h_index < (int)this->existing_network_vals.size(); h_index++) {
				this->existing_network_vals[h_index].erase(this->existing_network_vals[h_index].begin() + i_index);
				this->existing_network_is_on[h_index].erase(this->existing_network_is_on[h_index].begin() + i_index);
			}

			for (int h_index = 0; h_index < (int)this->new_existing_network_vals.size(); h_index++) {
				this->new_existing_network_vals[h_index].erase(this->new_existing_network_vals[h_index].begin() + i_index);
				this->new_existing_network_is_on[h_index].erase(this->new_existing_network_is_on[h_index].begin() + i_index);
			}
		}
	}

	for (int i_index = (int)this->new_network_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->new_network_inputs[i_index].scope_context.size(); l_index++) {
			if (this->new_network_inputs[i_index].scope_context[l_index] == scope
					&& this->new_network_inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->new_network_inputs.erase(this->new_network_inputs.begin() + i_index);
			this->new_network->remove_input(i_index);

			for (int h_index = 0; h_index < (int)this->new_network_vals.size(); h_index++) {
				this->new_network_vals[h_index].erase(this->new_network_vals[h_index].begin() + i_index);
				this->new_network_is_on[h_index].erase(this->new_network_is_on[h_index].begin() + i_index);
			}
		}
	}
}

void RefineExperiment::replace_obs_node(Scope* scope,
										int original_node_id,
										int new_node_id) {
	for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
		if (this->existing_network_inputs[i_index].scope_context.back() == scope
				&& this->existing_network_inputs[i_index].node_context.back() == original_node_id) {
			this->existing_network_inputs[i_index].node_context.back() = new_node_id;
		}
	}

	for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
		if (this->new_network_inputs[i_index].scope_context.back() == scope
				&& this->new_network_inputs[i_index].node_context.back() == original_node_id) {
			this->new_network_inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

RefineExperimentHistory::RefineExperimentHistory() {
	uniform_int_distribution<int> on_distribution(0, 99);
	if (on_distribution(generator) == 0) {
		this->is_on = true;
	} else {
		this->is_on = false;
	}
}

RefineExperimentState::RefineExperimentState(RefineExperiment* experiment) {
	this->experiment = experiment;
}
