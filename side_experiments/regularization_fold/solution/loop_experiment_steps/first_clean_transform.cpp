#include "loop_experiment.h"

#include <iostream>

#include "globals.h"
#include "layer.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "state_network.h"

using namespace std;

void LoopExperiment::first_clean_transform() {
	cout << "first_clean_transform" << endl;

	int input_size = (int)this->sequence->input_types.size();

	for (int i_index = 0; i_index < input_size; i_index++) {
		if (this->sequence->input_is_new_class[i_index]) {
			this->sequence->input_furthest_layer_needed_in[i_index] = (int)this->scope_context.size()+2;

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
					it != this->sequence->state_networks.end(); it++) {
				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in[it->first];
				int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						StateNetwork* network = it->second[n_index][i_index];
						double sum_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							if (abs(network->output->weights[0][0][in_index]) > 0.05) {
								sum_impact += abs(network->hidden->weights[in_index][0][0]);

								for (int s_index = 0; s_index < (int)network->state_indexes.size(); s_index++) {
									sum_impact += abs(network->hidden->weights[in_index][1][s_index]);
								}
							}
						}

						if (sum_impact > 0.1) {
							this->sequence->scope_additions_needed[i_index].insert(it->first);

							it->second[n_index][i_index]->clean(num_new_states);

							if (furthest_layer_seen_in < this->sequence->input_furthest_layer_needed_in[i_index]) {
								this->sequence->input_furthest_layer_needed_in[i_index] = furthest_layer_seen_in;
							}
						} else {
							delete it->second[n_index][i_index];
							it->second[n_index][i_index] = NULL;
						}
					}
				}
			}
		}
	}

	this->scope_node_additions_needed = vector<set<pair<int, int>>>(NUM_NEW_STATES);
	this->sequence->scope_node_additions_needed = vector<set<pair<int, int>>>(input_size);

	for (int l_index = 0; l_index < (int)this->scope_context.size()-1; l_index++) {
		Scope* scope = solution->scopes[this->scope_context[l_index]];
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->node_context[l_index]];
		for (int s_index = 0; s_index < scope->num_states; s_index++) {
			bool passed_in = false;
			for (int i_index = 0; i_index < (int)scope_node->input_indexes.size(); i_index++) {
				if (s_index == scope_node->input_indexes[i_index]) {
					passed_in = true;
					break;
				}
			}

			if (!passed_in) {
				this->corr_calc_scope_depths.push_back((int)this->scope_context.size()-1 - l_index);
				this->corr_calc_input_indexes.push_back(s_index);
				this->corr_calc_average_vals.push_back(0.0);
				this->corr_calc_variances.push_back(0.0);

				this->corr_calc_new_average_vals.push_back(vector<double>(NUM_NEW_STATES, 0.0));
				this->corr_calc_new_variances.push_back(vector<double>(NUM_NEW_STATES, 0.0));
				this->corr_calc_covariances.push_back(vector<double>(NUM_NEW_STATES, 0.0));
				this->new_transformations.push_back(vector<TransformationHelper>(NUM_NEW_STATES));

				this->sequence->corr_calc_new_average_vals.push_back(vector<double>(input_size, 0.0));
				this->sequence->corr_calc_new_variances.push_back(vector<double>(input_size, 0.0));
				this->sequence->corr_calc_covariances.push_back(vector<double>(input_size, 0.0));
				this->sequence->new_transformations.push_back(vector<TransformationHelper>(input_size));
			}
		}
	}
	{
		Scope* scope = solution->scopes[this->scope_context.back()];
		for (int s_index = 0; s_index < scope->num_states; s_index++) {
			this->corr_calc_scope_depths.push_back(0);
			this->corr_calc_input_indexes.push_back(s_index);
			this->corr_calc_average_vals.push_back(0.0);
			this->corr_calc_variances.push_back(0.0);

			this->corr_calc_new_average_vals.push_back(vector<double>(NUM_NEW_STATES, 0.0));
			this->corr_calc_new_variances.push_back(vector<double>(NUM_NEW_STATES, 0.0));
			this->corr_calc_covariances.push_back(vector<double>(NUM_NEW_STATES, 0.0));
			this->new_transformations.push_back(vector<TransformationHelper>(NUM_NEW_STATES));

			this->sequence->corr_calc_new_average_vals.push_back(vector<double>(input_size, 0.0));
			this->sequence->corr_calc_new_variances.push_back(vector<double>(input_size, 0.0));
			this->sequence->corr_calc_covariances.push_back(vector<double>(input_size, 0.0));
			this->sequence->new_transformations.push_back(vector<TransformationHelper>(input_size));
		}
	}

	this->sequence->corr_calc_state_average_vals = vector<double>(NUM_NEW_STATES, 0.0);
	this->sequence->corr_calc_state_variances = vector<double>(NUM_NEW_STATES, 0.0);
	this->sequence->corr_calc_input_average_vals = vector<vector<double>>(NUM_NEW_STATES, vector<double>(input_size, 0.0));
	this->sequence->corr_calc_input_variances = vector<vector<double>>(NUM_NEW_STATES, vector<double>(input_size, 0.0));
	this->sequence->corr_calc_new_covariances = vector<vector<double>>(NUM_NEW_STATES, vector<double>(input_size, 0.0));
	this->sequence->new_new_transformations = vector<vector<TransformationHelper>>(NUM_NEW_STATES, vector<TransformationHelper>(input_size));

	this->state = EXPERIMENT_STATE_SECOND_CLEAN;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
