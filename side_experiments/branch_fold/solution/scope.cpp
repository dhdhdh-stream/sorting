#include "scope.h"

#include <iostream>

using namespace std;

const double TARGET_MAX_UPDATE = 0.002;

BaseScope::BaseScope(int num_outputs) {
	this->type = SCOPE_TYPE_BASE;

	this->num_outputs = num_outputs;
}

BaseScope::~BaseScope() {
	// do nothing
}

Scope::Scope(vector<AbstractScope*> actions,
			 int num_inputs,
			 int num_outputs,
			 vector<vector<int>> input_sizes,
			 vector<vector<SmallNetwork*>> input_networks,
			 vector<int> scope_compressed_scope_sizes,
			 vector<int> scope_compressed_s_input_sizes,
			 vector<bool> is_simple_append,
			 vector<int> new_layer_sizes,
			 vector<Network*> obs_networks,
			 vector<vector<int>> outer_input_indexes,
			 vector<SmallNetwork*> score_networks,
			 vector<vector<int>> inner_input_sizes,
			 vector<vector<SmallNetwork*>> inner_input_networks,
			 vector<int> inner_compress_num_layers,
			 vector<int> inner_compress_new_sizes,
			 vector<Network*> inner_compression_networks,
			 vector<vector<int>> inner_compressed_scope_sizes,
			 vector<vector<int>> inner_compressed_s_input_sizes,
			 vector<int> end_compressed_scope_sizes,
			 vector<int> end_compressed_s_input_sizes) {
	this->type = SCOPE_TYPE_SCOPE;

	this->actions = actions;
	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
	this->scope_compressed_scope_sizes = scope_compressed_scope_sizes;
	this->scope_compressed_s_input_sizes = scope_compressed_s_input_sizes;
	this->is_simple_append = is_simple_append;
	this->new_layer_sizes = new_layer_sizes;
	this->obs_networks = obs_networks;
	this->outer_input_indexes = outer_input_indexes;
	this->score_networks = score_networks;
	this->inner_input_sizes = inner_input_sizes;
	this->inner_input_networks = inner_input_networks;
	this->inner_compress_num_layers = inner_compress_num_layers;
	this->inner_compress_new_sizes = inner_compress_new_sizes;
	this->inner_compression_networks = inner_compression_networks;
	this->inner_compressed_scope_sizes = inner_compressed_scope_sizes;
	this->inner_compressed_s_input_sizes = inner_compressed_s_input_sizes;
	this->end_compressed_scope_sizes = end_compressed_scope_sizes;
	this->end_compressed_s_input_sizes = end_compressed_s_input_sizes;

	this->outputs.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		this->outputs.push_back(0.0);
	}
}

Scope::Scope(Scope* original) {
	this->type = SCOPE_TYPE_SCOPE;

	this->actions.reserve(original->actions.size());
	for (int a_index = 0; a_index < (int)original->actions.size(); a_index++) {
		if (original->actions[a_index]->type == SCOPE_TYPE_BASE) {
			this->actions.push_back(new BaseScope(((BaseScope*)original->actions[a_index])->num_outputs));
		} else {
			this->actions.push_back(new Scope((Scope*)original->actions[a_index]));
		}
	}

	this->num_inputs = original->num_inputs;
	this->num_outputs = original->num_outputs;

	this->input_sizes = original->input_sizes;
	this->input_networks.reserve(original->input_networks.size());
	for (int a_index = 0; a_index < (int)original->input_networks.size(); a_index++) {
		this->input_networks.push_back(vector<SmallNetwork*>(original->input_networks[a_index].size()));
		for (int i_index = 0; i_index < (int)original->input_networks[a_index].size(); i_index++) {
			this->input_networks[a_index][i_index] = new SmallNetwork(original->input_networks[a_index][i_index]);
		}
	}
	this->scope_compressed_scope_sizes = original->scope_compressed_scope_sizes;
	this->scope_compressed_s_input_sizes = original->scope_compressed_s_input_sizes;

	this->is_simple_append = original->is_simple_append;
	this->new_layer_sizes = original->new_layer_sizes;
	this->obs_networks.reserve(original->obs_networks.size());
	for (int a_index = 0; a_index < (int)original->obs_networks.size(); a_index++) {
		if (original->obs_networks[a_index] == NULL) {
			this->obs_networks.push_back(NULL);
		} else {
			this->obs_networks.push_back(new Network(original->obs_networks[a_index]));
		}
	}

	this->outer_input_indexes = original->outer_input_indexes;

	this->score_networks.reserve(original->score_networks.size());
	for (int a_index = 0; a_index < (int)original->score_networks.size(); a_index++) {
		if (original->score_networks[a_index] == NULL) {
			this->score_networks.push_back(NULL);
		} else {
			this->score_networks.push_back(new SmallNetwork(original->score_networks[a_index]));
		}
	}

	this->inner_input_sizes = original->inner_input_sizes;
	this->inner_input_networks.reserve(original->inner_input_networks.size());
	for (int a_index = 0; a_index < (int)original->inner_input_networks.size(); a_index++) {
		this->inner_input_networks.push_back(original->inner_input_networks[a_index]);
		for (int i_index = 0; i_index < (int)original->inner_input_networks[a_index].size(); i_index++) {
			this->inner_input_networks[a_index][i_index] = new SmallNetwork(original->inner_input_networks[a_index][i_index]);
		}
	}

	this->inner_compress_num_layers = original->inner_compress_num_layers;
	this->inner_compress_new_sizes = original->inner_compress_new_sizes;
	this->inner_compression_networks.reserve(original->inner_compression_networks.size());
	for (int a_index = 0; a_index < (int)original->inner_compression_networks.size(); a_index++) {
		if (original->inner_compression_networks[a_index] == NULL) {
			this->inner_compression_networks.push_back(NULL);
		} else {
			this->inner_compression_networks.push_back(new Network(original->inner_compression_networks[a_index]));
		}
	}
	this->inner_compressed_scope_sizes = original->inner_compressed_scope_sizes;
	this->inner_compressed_s_input_sizes = original->inner_compressed_s_input_sizes;

	this->end_compressed_scope_sizes = original->end_compressed_scope_sizes;
	this->end_compressed_s_input_sizes = original->end_compressed_s_input_sizes;

	this->outputs.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		this->outputs.push_back(0.0);
	}
}

Scope::~Scope() {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}
	for (int a_index = 0; a_index < (int)this->input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
			delete this->input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->obs_networks.size(); a_index++) {
		if (this->obs_networks[a_index] != NULL) {
			delete this->obs_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->score_networks.size(); a_index++) {
		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			delete this->inner_input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_compression_networks.size(); a_index++) {
		if (this->inner_compression_networks[a_index] != NULL) {
			delete this->inner_compression_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->zero_scopes.size(); a_index++) {
		if (this->zero_scopes[a_index] != NULL) {
			delete this->zero_scopes[a_index];
		}
	}
}

void Scope::activate(vector<vector<double>>& flat_vals,
					 vector<double> inputs,
					 double& predicted_score) {
	vector<vector<double>> local_state_vals;
	vector<vector<double>> local_s_input_vals;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->activate(flat_vals[0]);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.push_back(new_scope);
				local_s_input_vals.push_back(vector<double>());

				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
					this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																		   local_s_input_vals[0]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				this->score_networks[a_index]->activate(local_state_vals.back(),
														local_s_input_vals.back());
				predicted_score += this->score_networks[a_index]->output->acti_vals[0];

				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						vector<double> compression_input;
						for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
							for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
								compression_input.push_back(local_state_vals[sc_index][st_index]);
							}
						}
						for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
							compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
						}
						this->inner_compression_networks[a_index]->activate(compression_input);
						vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
						for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
						}
						for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
						local_state_vals.push_back(new_scope);
						local_s_input_vals.push_back(vector<double>());
					} else {
						// obs used for score, then popped
						local_state_vals.pop_back();
						local_s_input_vals.pop_back();
					}
				}
			}
			flat_vals.erase(flat_vals.begin());
		} else {
			for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
				local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
			}

			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
				this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																 local_s_input_vals.back());
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			((Scope*)this->actions[a_index])->activate(flat_vals,
													   scope_input,
													   predicted_score);

			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
					local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				vector<double> compress_input;
				compress_input = local_state_vals.back();
				compress_input.insert(compress_input.end(),
					((Scope*)this->actions[a_index])->outputs.begin(),
					((Scope*)this->actions[a_index])->outputs.end());
				compress_input.insert(compress_input.end(),
					local_s_input_vals.back().begin(),
					local_s_input_vals.back().end());
				this->obs_networks[a_index]->activate(compress_input);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.pop_back();
				local_state_vals.push_back(new_scope);
				local_s_input_vals.pop_back();
				local_s_input_vals.push_back(vector<double>());
			}
		}
	}

	int outputs_index = 0;
	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
			outputs_index++;
		}
	}
}

void Scope::backprop(vector<double> input_errors,
					 vector<double>& output_errors,
					 double& predicted_score,
					 double target_val) {
	cout << "*** ENTERING SCOPE ***" << endl;

	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	output_errors.reserve(this->num_inputs);
	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
		output_errors.push_back(0.0);
	}

	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
		cout << "a_index: " << a_index << endl;

		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

						local_state_errors.pop_back();
						local_s_input_errors.pop_back();
						int input_index = 0;
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
							for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
								local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}

							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							cout << "local_s_input_errors appended" << endl;
						}
						for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
							local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
							input_index++;
						}
					} else {
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
					}
				}

				vector<double> score_errors{target_val - predicted_score};
				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
					local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
					this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
					if (local_s_input_errors.back().size() < this->score_networks[a_index]->s_input_input->errors.size()) {
						cout << "local smaller" << endl;
						cout << "local_s_input_errors.back().size(): " << local_s_input_errors.back().size() << endl;
						cout << "this->score_networks[a_index]->s_input_input->errors.size(): " << this->score_networks[a_index]->s_input_input->errors.size() << endl;
						cout << "this->inner_compressed_s_input_sizes[a_index].size(): " << this->inner_compressed_s_input_sizes[a_index].size() << endl;
						cout << "this->inner_compressed_s_input_sizes[a_index][0]: " << this->inner_compressed_s_input_sizes[a_index][0] << endl;
						cout << "this->inner_compress_num_layers[a_index]: " << this->inner_compress_num_layers[a_index] << endl;
						exit(1);
					}
					local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
					this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
				}
				predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
						local_s_input_errors[1].pop_back();
					}
					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
					local_s_input_errors[0].pop_back();
				}

				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				local_s_input_errors.pop_back();
			}
		} else {
			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
			vector<double> new_input_errors(scope_num_outputs);
			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
					local_state_errors.back().pop_back();
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
					this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
				}
				local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
				local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
				int input_index = 0;
				for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
					local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
				for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
					local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
			}

			vector<double> new_output_errors;
			((Scope*)this->actions[a_index])->backprop(new_input_errors,
													   new_output_errors,
													   predicted_score,
													   target_val);

			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
					new_output_errors.pop_back();
				}
				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
					this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
				}
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
					local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
					this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
				}
			}

			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
				output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
				local_s_input_errors[0].pop_back();
			}
		}
	}

	cout << "*** EXITING SCOPE ***" << endl;
}

void Scope::add_to_dictionary(vector<Scope*>& scope_dictionary) {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_SCOPE) {
			scope_dictionary.push_back(new Scope((Scope*)this->actions[a_index]));

			((Scope*)this->actions[a_index])->add_to_dictionary(scope_dictionary);
		}
	}
}

// void Scope::setup_zero_scopes() {
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			this->zero_scopes.push_back(NULL);
// 		} else {
// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				Scope* new_scope = new Scope((Scope*)this->actions[a_index]);
// 				this->zero_scopes.push_back(new_scope);
// 			} else {
// 				this->zero_scopes.push_back(NULL);
// 			}

// 			((Scope*)this->actions[a_index])->setup_zero_scopes();
// 		}
// 	}
// }

// void Scope::zero_train_activate(vector<vector<double>>& flat_vals,
// 								vector<double> inputs,
// 								double& predicted_score) {
// 	vector<vector<double>> local_state_vals;
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->activate(flat_vals[0]);
// 				vector<double> new_scope(this->new_layer_sizes[a_index]);
// 				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 				}
// 				local_state_vals.push_back(new_scope);

// 				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 				}

// 				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
// 					this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						local_state_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 					}
// 				}

// 				this->score_networks[a_index]->activate(local_state_vals.back());
// 				predicted_score += this->score_networks[a_index]->output->acti_vals[0];

// 				if (this->inner_compress_num_layers[a_index] > 0) {
// 					if (this->inner_compression_networks[a_index] != NULL) {
// 						vector<double> compression_input;
// 						for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
// 							for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 								compression_input.push_back(local_state_vals[sc_index][st_index]);
// 							}
// 						}
// 						this->inner_compression_networks[a_index]->activate(compression_input);
// 						vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
// 						for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
// 							new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
// 						}
// 						for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
// 							local_state_vals.pop_back();
// 						}
// 						local_state_vals.push_back(new_scope);
// 					} else {
// 						// obs used for score, then popped
// 						local_state_vals.pop_back();
// 					}
// 				}
// 			}
// 			flat_vals.erase(flat_vals.begin());
// 		} else {
// 			for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 				local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 			}

// 			vector<double> scope_input;
// 			for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
// 				this->input_networks[a_index][i_index]->activate(local_state_vals.back());
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 				}
// 			}

// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				vector<vector<double>> copy_flat_vals = flat_vals;
// 				double copy_predicted_score = predicted_score;	// though shouldn't matter
// 				vector<double> empty_scope_input(this->zero_scopes[a_index]->num_inputs, 0.0);
// 				this->zero_scopes[a_index]->activate(copy_flat_vals,
// 													 empty_scope_input,
// 													 copy_predicted_score);
// 			}

// 			((Scope*)this->actions[a_index])->zero_train_activate(flat_vals,
// 																  scope_input,
// 																  predicted_score);

// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				this->zero_scopes[a_index]->zero_train_backprop((Scope*)this->actions[a_index]);
// 			}

// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
// 					local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				vector<double> compress_input;
// 				compress_input = local_state_vals.back();
// 				compress_input.insert(compress_input.end(),
// 					((Scope*)this->actions[a_index])->outputs.begin(),
// 					((Scope*)this->actions[a_index])->outputs.end());
// 				this->obs_networks[a_index]->activate(compress_input);
// 				vector<double> new_scope(this->new_layer_sizes[a_index]);
// 				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 				}
// 				local_state_vals.pop_back();
// 				local_state_vals.push_back(new_scope);
// 			}
// 		}
// 	}

// 	int outputs_index = 0;
// 	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
// 		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
// 			outputs_index++;
// 		}
// 	}
// }

// void Scope::zero_train_backprop(Scope* original) {
// 	vector<vector<double>> local_state_errors;
// 	int input_errors_index = 0;
// 	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
// 		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
// 		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
// 			double error = original->outputs[input_errors_index] - this->outputs[input_errors_index];
// 			local_state_errors[sc_index][st_index] = error;
// 			input_errors_index++;
// 		}
// 	}

// 	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				if (this->inner_compression_networks[a_index] != NULL) {
// 					this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

// 					local_state_errors.pop_back();
// 					int input_index = 0;
// 					for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
// 						local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
// 						for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
// 							local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
// 							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
// 							input_index++;
// 						}
// 					}
// 				}

// 				vector<double> score_errors{original->score_networks[a_index]->output->acti_vals[0]
// 					- this->score_networks[a_index]->output->acti_vals[0]};
// 				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
// 				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->input->errors.size(); st_index++) {
// 					local_state_errors.back()[st_index] += this->score_networks[a_index]->input->errors[st_index];
// 					this->score_networks[a_index]->input->errors[st_index] = 0.0;
// 				}

// 				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_state_errors[1].back();
// 						local_state_errors[1].pop_back();
// 					}
// 					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
// 					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->input->errors[in_index];
// 						this->inner_input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 					}
// 				}

// 				// ignore outer_input_indexes

// 				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 			}
// 		} else {
// 			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
// 			vector<double> new_input_errors(scope_num_outputs);
// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
// 					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
// 					local_state_errors.back().pop_back();
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
// 					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
// 					this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
// 				}
// 				local_state_errors.push_back(vector<double>(
// 					this->obs_networks[a_index]->input->errors.size()-scope_num_outputs));
// 				for (int i_index = 0; i_index < (int)this->obs_networks[a_index]->input->errors.size()-scope_num_outputs; i_index++) {
// 					local_state_errors.back()[i_index] = this->obs_networks[a_index]->input->errors[i_index];
// 					this->obs_networks[a_index]->input->errors[i_index] = 0.0;
// 				}
// 			}

// 			vector<double> new_output_errors;
// 			((Scope*)this->actions[a_index])->zero_train_backprop(
// 				new_input_errors,
// 				new_output_errors,
// 				(Scope*)original->actions[a_index]);

// 			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> input_errors(this->input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
// 					new_output_errors.pop_back();
// 				}
// 				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
// 				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->input->errors[in_index];
// 					this->input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 				}
// 			}

// 			// ignore outer_input_indexes
// 		}
// 	}
// }

// void Scope::zero_train_backprop(vector<double> input_errors,
// 								std::vector<double>& output_errors,
// 								Scope* original) {
// 	vector<vector<double>> local_state_errors;
// 	int input_errors_index = 0;
// 	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
// 		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
// 		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
// 			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
// 			input_errors_index++;
// 		}
// 	}

// 	output_errors.reserve(this->num_inputs);
// 	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
// 		output_errors.push_back(0.0);
// 	}

// 	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				if (this->inner_compression_networks[a_index] != NULL) {
// 					this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

// 					local_state_errors.pop_back();
// 					int input_index = 0;
// 					for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
// 						local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
// 						for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
// 							local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
// 							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
// 							input_index++;
// 						}
// 					}
// 				}

// 				vector<double> score_errors{original->score_networks[a_index]->output->acti_vals[0]
// 					- this->score_networks[a_index]->output->acti_vals[0]};
// 				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
// 				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->input->errors.size(); st_index++) {
// 					local_state_errors.back()[st_index] += this->score_networks[a_index]->input->errors[st_index];
// 					this->score_networks[a_index]->input->errors[st_index] = 0.0;
// 				}

// 				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_state_errors[1].back();
// 						local_state_errors[1].pop_back();
// 					}
// 					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
// 					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->input->errors[in_index];
// 						this->inner_input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 					}
// 				}

// 				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
// 					output_errors[this->outer_input_indexes[a_index][i_index]] = local_state_errors[0].back();
// 					local_state_errors[0].pop_back();
// 				}

// 				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 			}
// 		} else {
// 			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
// 			vector<double> new_input_errors(scope_num_outputs);
// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
// 					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
// 					local_state_errors.back().pop_back();
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
// 					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
// 					this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
// 				}
// 				local_state_errors.push_back(vector<double>(
// 					this->obs_networks[a_index]->input->errors.size()-scope_num_outputs));
// 				for (int i_index = 0; i_index < (int)this->obs_networks[a_index]->input->errors.size()-scope_num_outputs; i_index++) {
// 					local_state_errors.back()[i_index] = this->obs_networks[a_index]->input->errors[i_index];
// 					this->obs_networks[a_index]->input->errors[i_index] = 0.0;
// 				}
// 			}

// 			vector<double> new_output_errors;
// 			((Scope*)this->actions[a_index])->zero_train_backprop(
// 				new_input_errors,
// 				new_output_errors,
// 				(Scope*)original->actions[a_index]);

// 			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> input_errors(this->input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
// 					new_output_errors.pop_back();
// 				}
// 				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
// 				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->input->errors[in_index];
// 					this->input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 				}
// 			}

// 			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
// 				output_errors[this->outer_input_indexes[a_index][i_index]] = local_state_errors[0].back();
// 				local_state_errors[0].pop_back();
// 			}
// 		}
// 	}
// }

// void Scope::add_to_zero_train_dictionary(vector<Scope*>& scope_dictionary) {
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_SCOPE) {
// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				scope_dictionary.push_back(new Scope(this->zero_scopes[a_index]));
// 			} else {
// 				scope_dictionary.push_back(new Scope((Scope*)this->actions[a_index]));
// 			}

// 			((Scope*)this->actions[a_index])->add_to_zero_train_dictionary(scope_dictionary);
// 		}
// 	}
// }

// void Scope::explore_activate_flat(vector<vector<double>>& flat_vals,
// 								  vector<double> inputs,
// 								  double& predicted_score,
// 								  vector<int> explore_path,
// 								  int explore_start_inclusive,
// 								  int explore_end_non_inclusive,
// 								  vector<vector<double>>& new_flat_vals) {
// 	// TODO: pass an after_explore variable and use zero_trained scopes

// 	vector<vector<double>> local_state_vals;

// 	if (explore_path.size() == 0) {
// 		for (int a_index = 0; a_index < explore_start_inclusive; a_index++) {
// 			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 				if (this->new_layer_sizes[a_index] > 0) {
// 					this->obs_networks[a_index]->activate(flat_vals[0]);
// 					vector<double> new_scope(this->new_layer_sizes[a_index]);
// 					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 					}
// 					local_state_vals.push_back(new_scope);

// 					// if (a_index == explore_start_inclusive-1) {
// 					// 	vector<double> local_input;
// 					// 	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
// 					// 		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 					// 			local_input.push_back(local_state_vals[sc_index][st_index]);
// 					// 		}
// 					// 	}
// 					// 	new_flat_vals.push_back(local_input);
// 					// }

// 					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 						local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 					}

// 					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
// 						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0]);
// 						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 							local_state_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 						}
// 					}

// 					this->score_networks[a_index]->activate(local_state_vals.back());
// 					predicted_score += this->score_networks[a_index]->output->acti_vals[0];

// 					if (this->inner_compress_num_layers[a_index] > 0) {
// 						if (this->inner_compression_networks[a_index] != NULL) {
// 							vector<double> compression_input;
// 							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
// 								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 									compression_input.push_back(local_state_vals[sc_index][st_index]);
// 								}
// 							}
// 							this->inner_compression_networks[a_index]->activate(compression_input);
// 							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
// 							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
// 								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
// 							}
// 							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
// 								local_state_vals.pop_back();
// 							}
// 							local_state_vals.push_back(new_scope);
// 						} else {
// 							// obs used for score, then popped
// 							local_state_vals.pop_back();
// 						}
// 					}
// 				}
// 				flat_vals.erase(flat_vals.begin());
// 			} else {
// 				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 				}

// 				vector<double> scope_input;
// 				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
// 					this->input_networks[a_index][i_index]->activate(local_state_vals.back());
// 					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 					}
// 				}

// 				((Scope*)this->actions[a_index])->activate(flat_vals,
// 														   scope_input,
// 														   predicted_score);

// 				if (this->is_simple_append[a_index]) {
// 					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
// 						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
// 					}
// 				} else {
// 					if (a_index == explore_start_inclusive-1) {
// 						new_flat_vals.push_back(((Scope*)this->actions[a_index])->outputs);
// 					}

// 					if (this->new_layer_sizes[a_index] > 0) {
// 						vector<double> compress_input;
// 						compress_input = local_state_vals.back();
// 						compress_input.insert(compress_input.end(),
// 							((Scope*)this->actions[a_index])->outputs.begin(),
// 							((Scope*)this->actions[a_index])->outputs.end());
// 						this->obs_networks[a_index]->activate(compress_input);
// 						vector<double> new_scope(this->new_layer_sizes[a_index]);
// 						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 						}
// 						local_state_vals.pop_back();
// 						local_state_vals.push_back(new_scope);
// 					}
// 				}
// 			}
// 		}

// 		new_flat_vals.push_back(flat_vals[0]);
// 		flat_vals.erase(flat_vals.begin());
// 		new_flat_vals.push_back(flat_vals[0]);
// 		flat_vals.erase(flat_vals.begin());
// 		new_flat_vals.push_back(flat_vals[0]);
// 		flat_vals.erase(flat_vals.begin());

// 		for (int a_index = explore_end_non_inclusive; a_index < (int)this->actions.size(); a_index++) {
// 			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 				if (this->new_layer_sizes[a_index] > 0) {
// 					this->obs_networks[a_index]->activate(flat_vals[0]);
// 					vector<double> new_scope(this->new_layer_sizes[a_index]);
// 					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 					}
// 					local_state_vals.push_back(new_scope);

// 					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 						local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 					}

// 					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
// 						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0]);
// 						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 							local_state_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 						}
// 					}

// 					this->score_networks[a_index]->activate(local_state_vals.back());
// 					predicted_score += this->score_networks[a_index]->output->acti_vals[0];

// 					if (this->inner_compress_num_layers[a_index] > 0) {
// 						if (this->inner_compression_networks[a_index] != NULL) {
// 							vector<double> compression_input;
// 							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
// 								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 									compression_input.push_back(local_state_vals[sc_index][st_index]);
// 								}
// 							}
// 							this->inner_compression_networks[a_index]->activate(compression_input);
// 							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
// 							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
// 								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
// 							}
// 							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
// 								local_state_vals.pop_back();
// 							}
// 							local_state_vals.push_back(new_scope);
// 						} else {
// 							// obs used for score, then popped
// 							local_state_vals.pop_back();
// 						}
// 					}
// 				}
// 				flat_vals.erase(flat_vals.begin());
// 			} else {
// 				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 				}

// 				vector<double> scope_input;
// 				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
// 					this->input_networks[a_index][i_index]->activate(local_state_vals.back());
// 					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 					}
// 				}

// 				((Scope*)this->actions[a_index])->activate(flat_vals,
// 														   scope_input,
// 														   predicted_score);

// 				if (this->is_simple_append[a_index]) {
// 					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
// 						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
// 					}
// 				} else {
// 					if (this->new_layer_sizes[a_index] > 0) {
// 						vector<double> compress_input;
// 						compress_input = local_state_vals.back();
// 						compress_input.insert(compress_input.end(),
// 							((Scope*)this->actions[a_index])->outputs.begin(),
// 							((Scope*)this->actions[a_index])->outputs.end());
// 						this->obs_networks[a_index]->activate(compress_input);
// 						vector<double> new_scope(this->new_layer_sizes[a_index]);
// 						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 						}
// 						local_state_vals.pop_back();
// 						local_state_vals.push_back(new_scope);
// 					}
// 				}
// 			}
// 		}
// 	} else {
// 		for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 				if (this->new_layer_sizes[a_index] > 0) {
// 					this->obs_networks[a_index]->activate(flat_vals[0]);
// 					vector<double> new_scope(this->new_layer_sizes[a_index]);
// 					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 					}
// 					local_state_vals.push_back(new_scope);

// 					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 						local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 					}

// 					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
// 						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0]);
// 						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 							local_state_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 						}
// 					}

// 					this->score_networks[a_index]->activate(local_state_vals.back());
// 					predicted_score += this->score_networks[a_index]->output->acti_vals[0];

// 					if (this->inner_compress_num_layers[a_index] > 0) {
// 						if (this->inner_compression_networks[a_index] != NULL) {
// 							vector<double> compression_input;
// 							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
// 								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 									compression_input.push_back(local_state_vals[sc_index][st_index]);
// 								}
// 							}
// 							this->inner_compression_networks[a_index]->activate(compression_input);
// 							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
// 							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
// 								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
// 							}
// 							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
// 								local_state_vals.pop_back();
// 							}
// 							local_state_vals.push_back(new_scope);
// 						} else {
// 							// obs used for score, then popped
// 							local_state_vals.pop_back();
// 						}
// 					}
// 				}
// 				flat_vals.erase(flat_vals.begin());
// 			} else {
// 				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 				}

// 				vector<double> scope_input;
// 				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
// 					this->input_networks[a_index][i_index]->activate(local_state_vals.back());
// 					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 					}
// 				}

// 				if (explore_path[0] == a_index) {
// 					new_flat_vals.push_back(local_state_vals[0]);

// 					vector<int> trimmed_explore_path(explore_path.begin()+1, explore_path.end());
// 					((Scope*)this->actions[a_index])->explore_activate_flat(
// 						flat_vals,
// 						scope_input,
// 						predicted_score,
// 						trimmed_explore_path,
// 						explore_start_inclusive,
// 						explore_end_non_inclusive,
// 						new_flat_vals);
// 				} else {
// 					((Scope*)this->actions[a_index])->activate(flat_vals,
// 															   scope_input,
// 															   predicted_score);
// 				}

// 				if (this->is_simple_append[a_index]) {
// 					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
// 						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
// 					}
// 				} else {
// 					if (this->new_layer_sizes[a_index] > 0) {
// 						vector<double> compress_input;
// 						compress_input = local_state_vals.back();
// 						compress_input.insert(compress_input.end(),
// 							((Scope*)this->actions[a_index])->outputs.begin(),
// 							((Scope*)this->actions[a_index])->outputs.end());
// 						this->obs_networks[a_index]->activate(compress_input);
// 						vector<double> new_scope(this->new_layer_sizes[a_index]);
// 						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 						}
// 						local_state_vals.pop_back();
// 						local_state_vals.push_back(new_scope);
// 					}
// 				}
// 			}
// 		}
// 	}

// 	// redo when using zero train
// 	int outputs_index = 0;
// 	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
// 		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
// 			outputs_index++;
// 		}
// 	}
// }

Scope* construct_scope_helper(vector<Node*> nodes,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<SmallNetwork*>& outer_input_networks,
							  int& compress_num_layers,
							  int& compress_new_size,
							  Network*& compression_network,
							  vector<int>& compressed_scope_sizes,
							  vector<int>& compressed_s_input_sizes) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	vector<int> process_scope_stack;
	int num_scopes = 0;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		int starting_num_scopes = num_scopes;
		if (nodes[n_index]->new_layer_size > 0) {
			num_scopes++;
			if (nodes[n_index]->compress_num_layers > 0) {
				num_scopes -= nodes[n_index]->compress_num_layers;
				if (nodes[n_index]->compress_new_size > 0) {
					num_scopes++;
				}
			}
		}
		int ending_num_scopes = num_scopes;

		if (ending_num_scopes > starting_num_scopes) {
			process_scope_stack.push_back(n_index);
		} else if (ending_num_scopes < starting_num_scopes) {
			int num_diff = starting_num_scopes - ending_num_scopes;
			for (int d_index = 0; d_index < num_diff; d_index++) {
				if (process_scope_stack.size() > 0) {
					scope_starts.push_back(process_scope_stack.back());
					scope_ends.push_back(n_index);
					process_scope_stack.pop_back();
				}
				// else end of scope
			}
		}
	}
	// num_scopes may be > 0, but ignore

	if (scope_starts.back() == 0 && scope_ends.back() == (int)nodes.size()-1) {
		// outer scope is curr scope
		scope_starts.pop_back();
		scope_ends.pop_back();
	}

	if (scope_starts.size() > 0) {
		int s_index = (int)scope_starts.size()-1;
		while (true) {
			if (s_index == 0) {
				break;
			}

			if (scope_starts[s_index] <= scope_starts[s_index-1]
					&& scope_ends[s_index] >= scope_ends[s_index-1]) {
				scope_starts.erase(scope_starts.begin()+s_index-1);
				scope_ends.erase(scope_ends.begin()+s_index-1);
			}
			s_index--;
		}
	}

	vector<AbstractScope*> actions;

	vector<vector<int>> input_sizes;
	vector<vector<SmallNetwork*>> input_networks;
	vector<int> scope_compressed_scope_sizes;
	vector<int> scope_compressed_s_input_sizes;

	vector<bool> is_simple_append;
	vector<int> new_layer_sizes;
	vector<Network*> obs_networks;

	vector<vector<int>> outer_input_indexes;

	vector<SmallNetwork*> score_networks;

	vector<vector<int>> inner_input_sizes;
	vector<vector<SmallNetwork*>> inner_input_networks;

	vector<int> inner_compress_num_layers;
	vector<int> inner_compress_new_sizes;
	vector<Network*> inner_compression_networks;
	vector<vector<int>> inner_compressed_scope_sizes;
	vector<vector<int>> inner_compressed_s_input_sizes;

	vector<int> end_compressed_scope_sizes;
	vector<int> end_compressed_s_input_sizes;

	int n_index = 0;
	int outer_input_index = 0;
	compress_num_layers = 0;
	compress_new_size = 0;
	compression_network = NULL;
	int num_outputs = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				actions.push_back(new BaseScope(nodes[n_index]->obs_size));

				// push empty
				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<SmallNetwork*>());
				scope_compressed_scope_sizes.push_back(0);
				scope_compressed_s_input_sizes.push_back(0);

				is_simple_append.push_back(false);
				new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
				if (nodes[n_index]->obs_network == NULL) {
					obs_networks.push_back(NULL);
				} else {
					obs_networks.push_back(new Network(nodes[n_index]->obs_network));
				}

				outer_input_indexes.push_back(vector<int>());
				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<SmallNetwork*>());
				for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
					if (nodes[n_index]->score_input_layer[i_index] - curr_layer == 0) {
						inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
						inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));
					} else {
						outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
						outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));

						if (nodes[n_index]->score_input_layer[i_index] - curr_layer == -1) {
							for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}
				for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
					if (nodes[n_index]->input_layer[i_index] - curr_layer == 0) {
						inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
						inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));
					} else {
						outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
						outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));

						if (nodes[n_index]->input_layer[i_index] - curr_layer == -1) {
							for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}

				if (nodes[n_index]->score_network == NULL) {
					score_networks.push_back(NULL);
				} else {
					score_networks.push_back(new SmallNetwork(nodes[n_index]->score_network));
				}

				inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
				inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
				if (nodes[n_index]->compression_network == NULL) {
					inner_compression_networks.push_back(NULL);
				} else {
					inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
				}
				inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
				inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);

				n_index++;
			} else {
				vector<Node*> subscope(nodes.begin()+scope_starts[s_index],
					nodes.begin()+scope_ends[s_index]+1);	// inclusive end

				vector<int> new_outer_input_layer;
				vector<int> new_outer_input_sizes;
				vector<SmallNetwork*> new_outer_input_networks;
				int new_compress_num_layers;
				int new_compress_num_size;
				Network* new_compression_network;
				vector<int> new_compressed_scope_sizes;
				vector<int> new_compressed_s_input_sizes;
				Scope* new_scope = construct_scope_helper(subscope,
														  curr_layer+1,
														  new_outer_input_layer,
														  new_outer_input_sizes,
														  new_outer_input_networks,
														  new_compress_num_layers,
														  new_compress_num_size,
														  new_compression_network,
														  new_compressed_scope_sizes,
														  new_compressed_s_input_sizes);
				actions.push_back(new_scope);

				if (scope_ends[s_index] == (int)nodes.size()-1) {
					num_outputs = new_scope->num_outputs+new_compressed_scope_sizes.back();
					end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(),
						new_scope->num_outputs+new_compressed_scope_sizes.back());
					scope_compressed_scope_sizes.push_back(new_compressed_scope_sizes.back());
					new_compressed_scope_sizes.pop_back();
					end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(),
						new_compressed_s_input_sizes.back());
					scope_compressed_s_input_sizes.push_back(new_compressed_s_input_sizes.back());
					new_compressed_s_input_sizes.pop_back();

					if (new_compress_num_layers == 1) {
						// don't need to update for parent

						is_simple_append.push_back(false);
						new_layer_sizes.push_back(new_compress_num_size);
						if (new_compression_network == NULL) {
							obs_networks.push_back(NULL);
						} else {
							obs_networks.push_back(new Network(new_compression_network));
						}
					} else {
						compress_num_layers = new_compress_num_layers-1;
						compress_new_size = new_compress_num_size;
						compression_network = new_compression_network;
						compressed_scope_sizes = new_compressed_scope_sizes;
						compressed_s_input_sizes = new_compressed_s_input_sizes;

						is_simple_append.push_back(true);
						new_layer_sizes.push_back(0);
						obs_networks.push_back(NULL);
					}
				} else {
					is_simple_append.push_back(false);
					new_layer_sizes.push_back(new_compress_num_size);
					if (new_compression_network == NULL) {
						obs_networks.push_back(NULL);

						scope_compressed_scope_sizes.push_back(0);
						scope_compressed_s_input_sizes.push_back(0);
					} else {
						obs_networks.push_back(new Network(new_compression_network));

						scope_compressed_scope_sizes.push_back(new_compressed_scope_sizes.back());
						new_compressed_scope_sizes.pop_back();
						scope_compressed_s_input_sizes.push_back(new_compressed_s_input_sizes.back());
						new_compressed_s_input_sizes.pop_back();
					}
				}

				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<SmallNetwork*>());
				outer_input_indexes.push_back(vector<int>());
				for (int i_index = 0; i_index < (int)new_outer_input_networks.size(); i_index++) {
					// new_outer_input_layer[i_index] < 0
					if (new_outer_input_layer[i_index] == -1) {
						input_sizes.back().push_back(new_outer_input_sizes[i_index]);
						input_networks.back().push_back(new_outer_input_networks[i_index]);
					} else {
						outer_input_layer.push_back(new_outer_input_layer[i_index] + 1);
						outer_input_sizes.push_back(new_outer_input_sizes[i_index]);
						outer_input_networks.push_back(new_outer_input_networks[i_index]);

						if (new_outer_input_layer[i_index] + 1 == -1) {
							for (int is_index = 0; is_index < new_outer_input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}

				score_networks.push_back(NULL);

				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<SmallNetwork*>());

				inner_compress_num_layers.push_back(0);
				inner_compress_new_sizes.push_back(0);
				inner_compression_networks.push_back(NULL);
				inner_compressed_scope_sizes.push_back(vector<int>());
				inner_compressed_s_input_sizes.push_back(vector<int>());

				n_index = scope_ends[s_index]+1;

				break;
			}
		}
	}
	while (true) {
		if (n_index >= (int)nodes.size()) {
			break;
		}

		actions.push_back(new BaseScope(nodes[n_index]->obs_size));

		// push empty
		input_sizes.push_back(vector<int>());
		input_networks.push_back(vector<SmallNetwork*>());
		scope_compressed_scope_sizes.push_back(0);
		scope_compressed_s_input_sizes.push_back(0);

		is_simple_append.push_back(false);
		new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
		if (nodes[n_index]->obs_network == NULL) {
			obs_networks.push_back(NULL);
		} else {
			obs_networks.push_back(new Network(nodes[n_index]->obs_network));
		}

		outer_input_indexes.push_back(vector<int>());
		inner_input_sizes.push_back(vector<int>());
		inner_input_networks.push_back(vector<SmallNetwork*>());
		for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
			if (nodes[n_index]->score_input_layer[i_index] - curr_layer == 0) {
				inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
				inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));
			} else {
				outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
				outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));

				if (nodes[n_index]->score_input_layer[i_index] - curr_layer == -1) {
					for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
						outer_input_indexes.back().push_back(outer_input_index);
						outer_input_index++;
					}
				}
			}
		}
		for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
			if (nodes[n_index]->input_layer[i_index] - curr_layer == 0) {
				inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
				inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));
			} else {
				outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
				outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));

				if (nodes[n_index]->input_layer[i_index] - curr_layer == -1) {
					for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
						outer_input_indexes.back().push_back(outer_input_index);
						outer_input_index++;
					}
				}
			}
		}

		if (nodes[n_index]->score_network == NULL) {
			score_networks.push_back(NULL);
		} else {
			score_networks.push_back(new SmallNetwork(nodes[n_index]->score_network));
		}

		if (n_index == (int)nodes.size()-1) {
			if (nodes[n_index]->compress_num_layers == 0
					|| nodes[n_index]->compress_num_layers == 1
					|| (nodes[n_index]->compress_num_layers == 2 && nodes[n_index]->compress_new_size > 0)) {
				// edge case
				inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
				inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
				// nodes[n_index]->compress_new_size > 0
				if (nodes[n_index]->compression_network == NULL) {
					inner_compression_networks.push_back(NULL);
				} else {
					inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
				}
				inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
				inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);
			} else {
				// output last 2 layers, and pass the rest
				compress_num_layers = nodes[n_index]->compress_num_layers-2;
				compress_new_size = nodes[n_index]->compress_new_size;
				compression_network = nodes[n_index]->compression_network;
				compressed_scope_sizes = nodes[n_index]->compressed_scope_sizes;
				compressed_s_input_sizes = nodes[n_index]->compressed_s_input_sizes;

				num_outputs += compressed_scope_sizes.back();
				end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(), compressed_scope_sizes.back());
				compressed_scope_sizes.pop_back();
				end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(), compressed_s_input_sizes.back());
				compressed_s_input_sizes.pop_back();
				num_outputs += compressed_scope_sizes.back();
				end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(), compressed_scope_sizes.back());
				compressed_scope_sizes.pop_back();
				end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(), compressed_s_input_sizes.back());
				compressed_s_input_sizes.pop_back();

				inner_compress_num_layers.push_back(0);
				inner_compress_new_sizes.push_back(0);
				inner_compression_networks.push_back(NULL);
				inner_compressed_scope_sizes.push_back(vector<int>());
				inner_compressed_s_input_sizes.push_back(vector<int>());
			}
		} else {
			inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
			inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
			if (nodes[n_index]->compression_network == NULL) {
				inner_compression_networks.push_back(NULL);
			} else {
				inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
			}
			inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
			inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);
		}

		n_index++;
	}

	Scope* scope = new Scope(actions,
							 outer_input_index,
							 num_outputs,
							 input_sizes,
							 input_networks,
							 scope_compressed_scope_sizes,
							 scope_compressed_s_input_sizes,
							 is_simple_append,
							 new_layer_sizes,
							 obs_networks,
							 outer_input_indexes,
							 score_networks,
							 inner_input_sizes,
							 inner_input_networks,
							 inner_compress_num_layers,
							 inner_compress_new_sizes,
							 inner_compression_networks,
							 inner_compressed_scope_sizes,
							 inner_compressed_s_input_sizes,
							 end_compressed_scope_sizes,
							 end_compressed_s_input_sizes);

	return scope;
}

Scope* construct_scope(vector<Node*> nodes) {
	// will be empty
	vector<int> outer_input_layer;
	vector<int> outer_input_sizes;
	vector<SmallNetwork*> outer_input_networks;
	int compress_num_layers;
	int compress_num_size;
	Network* compression_network;
	vector<int> compressed_scope_sizes;
	vector<int> compressed_s_input_sizes;

	Scope* scope = construct_scope_helper(nodes,
										  -1,	// start at -1 because outer scope has no state
										  outer_input_layer,
										  outer_input_sizes,
										  outer_input_networks,
										  compress_num_layers,
										  compress_num_size,
										  compression_network,
										  compressed_scope_sizes,
										  compressed_s_input_sizes);

	return scope;
}
