#include "branch_path.h"

#include <iostream>

using namespace std;



void BranchPath::activate(vector<vector<double>>& flat_vals,
						  vector<double>& local_state_vals,
						  vector<double>& local_s_input_vals,
						  vector<double>& output_state_vals,
						  double& predicted_score,
						  double& scale_factor) {
	for (int a_index = 0; a_index < (int)this->scopes.size(); a_index++) {
		vector<double> new_state_vals;

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			new_state_vals = flat_vals.begin();
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate(local_state_vals,
																	   input);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			predicted_score += this->scope_average_mod[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;

			vector<double> scope_output;
			scope_scope->activate(flat_vals,
								  temp_new_s_input_vals,
								  scope_output,
								  predicted_score,
								  scale_factor);

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals = scope_output;
		}

		if (this->need_process[a_index]) {
			vector<double> combined_state_vals = local_state_vals;
			combined_state_vals.insert(combined_state_vals.end(),
				new_state_vals.begin(), new_state_vals.end());

			if (this->is_branch[a_index]) {
				vector<double> output_state_vals;
				this->branches[a_index]->activate(combined_state_vals,
												  input,
												  output_state_vals,
												  flat_vals,
												  predicted_score,
												  scale_factor);
				local_state_vals = output_state_vals;
			} else {
				this->score_networks[a_index]->activate_small(combined_state_vals,
															  inputs);
				predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

				if (this->active_compress[a_index]) {
					// compress 2 layers, add 1 layer
					this->compress_networks[a_index]->activate_small(combined_state_vals,
																	 inputs);

					local_state_vals.clear();
					local_state_vals.reserve(this->compress_new_sizes[a_index]);
					for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				}
				// else just let new_state_vals go
			}
		}
	}

	this->end_input_network->activate(local_state_vals,
									  local_s_input_vals);
	output_state_vals.reserve(this->end_input_size);
	for (int i_index = 0; i_index < this->end_input_size; i_index++) {
		output_state_vals.push_back(this->end_input_network->output->acti_vals[i_index]);
	}
}
