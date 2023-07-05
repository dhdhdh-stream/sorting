#include "branch_experiment.h"

using namespace std;

void BranchExperiment::new_types_transform() {
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			int input_size = this->sequences[a_index]->input_init_types.size();
			for (int i_index = 0; i_index < input_size; i_index++) {
				if (this->sequences[a_index]->input_is_new_class[i_index]) {
					this->sequences[a_index]->input_furthest_layer_seen_in[i_index] = this->scope_context.size()+2;

					for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
							it != this->sequences[a_index]->state_networks.end(); it++) {
						int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
						int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

						for (int n_index = 0; n_index < it->second.size(); n_index++) {
							if (it->second[n_index].size() > 0) {
								double sum_obs_impact = 0.0;
								for (int in_index = 0; in_index < 20; in_index++) {
									sum_obs_impact += abs(it->second[n_index][i_index]->hidden->weights[in_index][0][0]);
								}

								if (sum_obs_impact > 0.1) {
									it->second[n_index][i_index]->clean(num_new_states);

									if (furthest_layer_seen_in < this->sequences[a_index]->input_furthest_layer_seen_in[i_index]) {
										this->sequences[a_index]->input_furthest_layer_seen_in[i_index] = furthest_layer_seen_in;
									}
								} else {
									delete it->second[n_index][i_index];
									it->second[n_index][i_index] = NULL;
								}
							}
						}
					}

					for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
						if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
							double sum_obs_impact = 0.0;
							for (int in_index = 0; in_index < 20; in_index++) {
								sum_obs_impact += abs(this->sequences[a_index]->step_state_networks[ia_index][i_index]->hidden->weights[in_index][0][0]);
							}

							if (sum_obs_impact > 0.1) {
								this->sequences[a_index]->step_state_networks[ia_index][i_index]->clean(this->layer_num_new_states.back());

								if (this->scopes.size()+1 < this->sequences[a_index]->input_furthest_layer_seen_in[i_index]) {
									this->sequences[a_index]->input_furthest_layer_seen_in[i_index] = this->scopes.size()+1;
								}
							} else {
								delete this->sequences[a_index]->step_state_networks[ia_index][i_index];
								this->sequences[a_index]->step_state_networks[ia_index][i_index] = NULL;
							}
						} else {
							// this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE
							for (int s_index = 0; s_index < (int)this->sequences[ia_index]->scopes.size(); s_index++) {
								Scope* scope = solution->scopes[this->sequences[ia_index]->scopes[s_index]];
								for (int n_index = 0; n_index < (int)this->sequences[ia_index]->node_ids[s_index].size(); n_index++) {
									if (scope->nodes[this->sequences[ia_index]->node_ids[s_index][n_index]]->type == NODE_TYPE_ACTION) {
										double sum_obs_impact = 0.0;
										for (int in_index = 0; in_index < 20; in_index++) {
											sum_obs_impact += abs(this->sequences[a_index]->sequence_state_networks
												[ia_index][s_index][n_index][i_index]->hidden->weights[in_index][0][0]);
										}

										if (sum_obs_impact > 0.1) {
											this->sequences[a_index]->sequence_state_networks[ia_index][s_index][n_index][i_index]->clean(this->layer_num_new_states.back());

											if (this->scopes.size()+1 < this->sequences[a_index]->input_furthest_layer_seen_in[i_index]) {
												this->sequences[a_index]->input_furthest_layer_seen_in[i_index] = this->scopes.size()+1;
											}
										} else {
											delete this->sequences[a_index]->sequence_state_networks[ia_index][s_index][n_index][i_index];
											this->sequences[a_index]->sequence_state_networks[ia_index][s_index][n_index][i_index] = NULL;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// TODO: add new state everywhere and begin updating score networks?
	// - only for new states, not for new inputs?
	//   - new inputs will gradually take effect after experiment?
	// - "finalize" temporary score networks too
	//   - move states

	this->state = EXPERIMENT_STATE_CLEANUP;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
