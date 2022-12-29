#include "finished_step.h"

#include <iostream>

#include "definitions.h"

using namespace std;

FinishedStep::FinishedStep(bool is_inner_scope,
						   Scope* scope,
						   int obs_size,
						   vector<int> inner_input_input_layer,
						   vector<int> inner_input_input_sizes,
						   vector<FoldNetwork*> inner_input_input_networks,
						   FoldNetwork* inner_input_network,
						   double scope_scale_mod,
						   FoldNetwork* score_network,
						   int compress_num_layers,
						   int compress_new_size,
						   FoldNetwork* compress_network,
						   int compress_original_size,
						   vector<int> compressed_s_input_sizes,
						   vector<int> compressed_scope_sizes,
						   vector<int> input_layer,
						   vector<int> input_sizes,
						   vector<FoldNetwork*> input_networks) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->is_inner_scope = is_inner_scope;
	this->scope = scope;
	this->obs_size = obs_size;

	this->inner_input_input_layer = inner_input_input_layer;
	this->inner_input_input_sizes = inner_input_input_sizes;
	this->inner_input_input_networks = inner_input_input_networks;
	this->inner_input_network = inner_input_network;
	this->scope_scale_mod = scope_scale_mod;

	this->score_network = score_network;

	this->average_misguess = 0.0;
	this->average_inner_scope_impact = 0.0;
	this->average_local_impact = 0.0;

	this->compress_num_layers = compress_num_layers;
	this->compress_new_size = compress_new_size;
	if (this->compress_num_layers > 0 && this->compress_new_size > 0) {
		this->active_compress = true;
	} else {
		this->active_compress = false;
	}
	this->compress_network = compress_network;
	this->compress_original_size = compress_original_size;

	this->compressed_s_input_sizes = compressed_s_input_sizes;
	this->compressed_scope_sizes = compressed_scope_sizes;

	this->input_layer = input_layer;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
}

FinishedStep::FinishedStep(FinishedStep* original) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->is_inner_scope = original->is_inner_scope;
	this->scope = new Scope(original->scope);
	this->obs_size = original->obs_size;

	this->inner_input_input_layer = original->inner_input_input_layer;
	this->inner_input_input_sizes = original->inner_input_input_sizes;
	for (int i_index = 0; i_index < (int)original->inner_input_input_networks.size(); i_index++) {
		this->inner_input_input_networks.push_back(new FoldNetwork(original->inner_input_input_networks[i_index]));
	}
	this->inner_input_network = new FoldNetwork(original->inner_input_network);
	this->scope_scale_mod = original->scope_scale_mod;

	this->score_network = new FoldNetwork(original->score_network);

	this->average_misguess = original->average_misguess;
	this->average_inner_scope_impact = original->average_inner_scope_impact;
	this->average_local_impact = original->average_local_impact;

	this->compress_num_layers = original->compress_num_layers;
	this->active_compress = original->active_compress;
	this->compress_new_size = original->compress_new_size;
	this->compress_network = new FoldNetwork(original->compress_network);
	this->compress_original_size = original->compress_original_size;

	this->compressed_s_input_sizes = original->compressed_s_input_sizes;
	this->compressed_scope_sizes = original->compressed_scope_sizes;

	this->input_layer = original->input_layer;
	this->input_sizes = original->input_sizes;
	for (int i_index = 0; i_index < (int)original->input_networks.size(); i_index++) {
		this->input_networks.push_back(new FoldNetwork(original->input_networks[i_index]));
	}
}

FinishedStep::FinishedStep(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string is_inner_scope_line;
	getline(input_file, is_inner_scope_line);
	this->is_inner_scope = stoi(is_inner_scope_line);

	if (this->is_inner_scope) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(scope_id) + ".txt");
		this->scope = new Scope(scope_save_file);
		scope_save_file.close();
	} else {
		this->scope = NULL;
	}

	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	if (this->is_inner_scope) {
		string inner_input_input_networks_size_line;
		getline(input_file, inner_input_input_networks_size_line);
		int inner_input_input_networks_size = stoi(inner_input_input_networks_size_line);

		for (int i_index = 0; i_index < inner_input_input_networks_size; i_index++) {
			string inner_input_input_layer_line;
			getline(input_file, inner_input_input_layer_line);
			this->inner_input_input_layer.push_back(stoi(inner_input_input_layer_line));

			string inner_input_input_size_line;
			getline(input_file, inner_input_input_size_line);
			this->inner_input_input_sizes.push_back(stoi(inner_input_input_size_line));

			ifstream inner_input_input_network_save_file;
			inner_input_input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_inner_input_input_" + to_string(i_index) + ".txt");
			this->inner_input_input_networks.push_back(new FoldNetwork(inner_input_input_network_save_file));
			inner_input_input_network_save_file.close();
		}

		ifstream inner_input_network_save_file;
		inner_input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_inner_input.txt");
		this->inner_input_network = new FoldNetwork(inner_input_network_save_file);
		inner_input_network_save_file.close();

		string scope_scale_mod_line;
		getline(input_file, scope_scale_mod_line);
		this->scope_scale_mod = stof(scope_scale_mod_line);
	} else {
		this->inner_input_network = NULL;
	}

	ifstream score_network_save_file;
	score_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_score.txt");
	this->score_network = new FoldNetwork(score_network_save_file);
	score_network_save_file.close();

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);

	string average_inner_scope_impact_line;
	getline(input_file, average_inner_scope_impact_line);
	this->average_inner_scope_impact = stof(average_inner_scope_impact_line);

	string average_local_impact_line;
	getline(input_file, average_local_impact_line);
	this->average_local_impact = stof(average_local_impact_line);

	string compress_num_layers_line;
	getline(input_file, compress_num_layers_line);
	this->compress_num_layers = stoi(compress_num_layers_line);

	string active_compress_line;
	getline(input_file, active_compress_line);
	this->active_compress = stoi(active_compress_line);

	string compress_new_size_line;
	getline(input_file, compress_new_size_line);
	this->compress_new_size = stoi(compress_new_size_line);

	if (this->active_compress) {
		ifstream compress_network_save_file;
		compress_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_compress.txt");
		this->compress_network = new FoldNetwork(compress_network_save_file);
		compress_network_save_file.close();
	}

	string compress_original_size_line;
	getline(input_file, compress_original_size_line);
	this->compress_original_size = stoi(compress_original_size_line);

	for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
		string compressed_s_input_size_line;
		getline(input_file, compressed_s_input_size_line);
		this->compressed_s_input_sizes.push_back(stoi(compressed_s_input_size_line));

		string compressed_scope_size_line;
		getline(input_file, compressed_scope_size_line);
		this->compressed_scope_sizes.push_back(stoi(compressed_scope_size_line));
	}

	string input_networks_size_line;
	getline(input_file, input_networks_size_line);
	int input_networks_size = stoi(input_networks_size_line);

	for (int i_index = 0; i_index < input_networks_size; i_index++) {
		string input_layer_line;
		getline(input_file, input_layer_line);
		this->input_layer.push_back(stoi(input_layer_line));

		string input_size_line;
		getline(input_file, input_size_line);
		this->input_sizes.push_back(stoi(input_size_line));

		ifstream input_network_save_file;
		input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_input_" + to_string(i_index) + ".txt");
		this->input_networks.push_back(new FoldNetwork(input_network_save_file));
		input_network_save_file.close();
	}
}

FinishedStep::~FinishedStep() {
	for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
		delete this->inner_input_input_networks[i_index];
	}
	delete this->inner_input_network;

	delete this->score_network;

	delete this->compress_network;

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		delete this->input_networks[i_index];
	}
}

void FinishedStep::explore_on_path_activate(vector<vector<double>>& flat_vals,
											vector<vector<double>>& s_input_vals,
											vector<vector<double>>& state_vals,
											double& predicted_score,
											double& scale_factor,
											FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		int explore_phase = EXPLORE_PHASE_NONE;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->explore_off_path_activate(flat_vals,
											   scope_input,
											   scope_output,
											   predicted_score,
											   scale_factor,
											   explore_phase,
											   scope_history);
		history->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_vals.size()-1;
	}
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals);
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

// cannot be explore_on_path_backprop for FinishedStep

void FinishedStep::explore_off_path_activate(vector<vector<double>>& flat_vals,
											 vector<vector<double>>& s_input_vals,
											 vector<vector<double>>& state_vals,
											 double& predicted_score,
											 double& scale_factor,
											 int& explore_phase,
											 FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* inner_input_input_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
				this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																		  state_vals[this->inner_input_input_layer[i_index]],
																		  inner_input_input_history);
				history->inner_input_input_network_histories.push_back(inner_input_input_history);
			} else {
				this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																		  state_vals[this->inner_input_input_layer[i_index]]);
			}
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_network);
			this->inner_input_network->activate_small(s_input_vals.back(),
													  state_vals.back(),
													  inner_input_network_history);
			history->inner_input_network_history = inner_input_network_history;
		} else {
			this->inner_input_network->activate_small(s_input_vals.back(),
													  state_vals.back());
		}
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->explore_off_path_activate(flat_vals,
											   scope_input,
											   scope_output,
											   predicted_score,
											   scale_factor,
											   explore_phase,
											   scope_history);
		history->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* input_network_history = new FoldNetworkHistory(this->input_networks[i_index]);
			this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
														  state_vals[this->input_layer[i_index]],
														  input_network_history);
			history->input_network_histories.push_back(input_network_history);
		} else {
			this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
														  state_vals[this->input_layer[i_index]]);
		}
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_vals.size()-1;
	}
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
		this->score_network->activate_subfold(s_input_vals[s_input_index],
											  state_vals,
											  score_network_history);
		history->score_network_history = score_network_history;
	} else {
		this->score_network->activate_subfold(s_input_vals[s_input_index],
											  state_vals);
	}
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_network);
				this->compress_network->activate_subfold(s_input_vals[s_input_index],
														 state_vals,
														 compress_network_history);
				history->compress_network_history = compress_network_history;
			} else {
				this->compress_network->activate_subfold(s_input_vals[s_input_index],
														 state_vals);
			}
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::explore_off_path_backprop(vector<vector<double>>& s_input_errors,
											 vector<vector<double>>& state_errors,
											 double& predicted_score,
											 double target_val,
											 double& scale_factor,
											 double& scale_factor_error,
											 FinishedStepHistory* history) {
	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			state_errors.push_back(vector<double>(this->compressed_scope_sizes[0], 0.0));
			for (int l_index = 1; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[s_input_errors.size()-this->compress_num_layers].size(); s_index++) {
				s_input_errors[s_input_errors.size()-this->compress_num_layers][s_index] += this->compress_network->s_input_input->errors[s_index];
				this->compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->compress_network->state_inputs[l_index]->errors[s_index];
					this->compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	double predicted_score_error = target_val - predicted_score;

	scale_factor_error += history->score_update*predicted_score_error;

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_errors.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_errors.size()-1;
	}
	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[s_input_index].size(); s_index++) {
		s_input_errors[s_input_index][s_index] += this->score_network->s_input_input->errors[s_index];
		this->score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = s_input_index; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->score_network->state_inputs[l_index]->errors[s_index];
			this->score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*history->score_update;

	for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
		vector<double> input_errors(this->input_sizes[i_index]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
			s_input_errors[this->input_layer[i_index]+1].pop_back();
		}
		vector<double> s_input_output_errors;
		vector<double> state_output_errors;
		this->input_networks[i_index]->backprop_small_errors_with_no_weight_change(
			input_errors,
			s_input_output_errors,
			state_output_errors,
			history->input_network_histories[i_index]);
		for (int s_index = 0; s_index < (int)s_input_errors[this->input_layer[i_index]].size(); s_index++) {
			s_input_errors[this->input_layer[i_index]][s_index] += s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors[this->input_layer[i_index]].size(); s_index++) {
			state_errors[this->input_layer[i_index]][s_index] += state_output_errors[s_index];
		}
	}

	if (!this->is_inner_scope) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> existing_scope_output_errors = s_input_errors.back();
		s_input_errors.pop_back();
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scope->explore_off_path_backprop(scope_input_errors,
											   scope_output_errors,
											   predicted_score,
											   target_val,
											   scale_factor,
											   scope_scale_factor_error,
											   history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;

		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_output_errors[s_index] += existing_scope_output_errors[s_index];
		}

		vector<double> inner_input_s_input_output_errors;
		vector<double> inner_input_state_output_errors;
		this->inner_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			inner_input_s_input_output_errors,
			inner_input_state_output_errors,
			history->inner_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += inner_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += inner_input_state_output_errors[s_index];
		}

		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> inner_input_input_errors(this->inner_input_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				inner_input_input_errors[this->inner_input_input_sizes[i_index]-1-s_index] = s_input_errors[this->inner_input_input_layer[i_index]+1].back();
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->inner_input_input_networks[i_index]->backprop_small_errors_with_no_weight_change(
				inner_input_input_errors,
				s_input_output_errors,
				state_output_errors,
				history->inner_input_input_network_histories[i_index]);
			for (int s_index = 0; s_index < (int)s_input_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]][s_index] += s_input_output_errors[s_index];
			}
			for (int s_index = 0; s_index < (int)state_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				state_errors[this->inner_input_input_layer[i_index]][s_index] += state_output_errors[s_index];
			}
		}
	}
}

void FinishedStep::existing_flat_activate(vector<vector<double>>& flat_vals,
										  vector<vector<double>>& s_input_vals,
										  vector<vector<double>>& state_vals,
										  double& predicted_score,
										  double& scale_factor,
										  FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			FoldNetworkHistory* inner_input_input_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]],
																	  inner_input_input_history);
			history->inner_input_input_network_histories.push_back(inner_input_input_history);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_network);
		this->inner_input_network->activate_small(s_input_vals.back(),
													   state_vals.back(),
													   inner_input_network_history);
		history->inner_input_network_history = inner_input_network_history;
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->existing_flat_activate(flat_vals,
											scope_input,
											scope_output,
											predicted_score,
											scale_factor,
											scope_history);
		history->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		FoldNetworkHistory* input_network_history = new FoldNetworkHistory(this->input_networks[i_index]);
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]],
													  input_network_history);
		history->input_network_histories.push_back(input_network_history);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_vals.size()-1;
	}
	FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals,
										  score_network_history);
	history->score_network_history = score_network_history;
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_network);
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals,
													 compress_network_history);
			history->compress_network_history = compress_network_history;
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::existing_flat_backprop(vector<vector<double>>& s_input_errors,
										  vector<vector<double>>& state_errors,
										  double& predicted_score,
										  double predicted_score_error,
										  double& scale_factor,
										  double& scale_factor_error,
										  FinishedStepHistory* history) {
	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			state_errors.push_back(vector<double>(this->compressed_scope_sizes[0], 0.0));
			for (int l_index = 1; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[s_input_errors.size()-this->compress_num_layers].size(); s_index++) {
				s_input_errors[s_input_errors.size()-this->compress_num_layers][s_index] += this->compress_network->s_input_input->errors[s_index];
				this->compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->compress_network->state_inputs[l_index]->errors[s_index];
					this->compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	scale_factor_error += history->score_update*predicted_score_error;

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_errors.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_errors.size()-1;
	}
	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[s_input_index].size(); s_index++) {
		s_input_errors[s_input_index][s_index] += this->score_network->s_input_input->errors[s_index];
		this->score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = s_input_index; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->score_network->state_inputs[l_index]->errors[s_index];
			this->score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*history->score_update;

	for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
		vector<double> input_errors(this->input_sizes[i_index]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
			s_input_errors[this->input_layer[i_index]+1].pop_back();
		}
		vector<double> s_input_output_errors;
		vector<double> state_output_errors;
		this->input_networks[i_index]->backprop_small_errors_with_no_weight_change(
			input_errors,
			s_input_output_errors,
			state_output_errors,
			history->input_network_histories[i_index]);
		for (int s_index = 0; s_index < (int)s_input_errors[this->input_layer[i_index]].size(); s_index++) {
			s_input_errors[this->input_layer[i_index]][s_index] += s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors[this->input_layer[i_index]].size(); s_index++) {
			state_errors[this->input_layer[i_index]][s_index] += state_output_errors[s_index];
		}
	}

	if (!this->is_inner_scope) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> existing_scope_output_errors = s_input_errors.back();
		s_input_errors.pop_back();
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scope->existing_flat_backprop(scope_input_errors,
											scope_output_errors,
											predicted_score,
											predicted_score_error,
											scale_factor,
											scope_scale_factor_error,
											history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;

		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_output_errors[s_index] += existing_scope_output_errors[s_index];
		}

		vector<double> inner_input_s_input_output_errors;
		vector<double> inner_input_state_output_errors;
		this->inner_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			inner_input_s_input_output_errors,
			inner_input_state_output_errors,
			history->inner_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += inner_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += inner_input_state_output_errors[s_index];
		}

		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> inner_input_input_errors(this->inner_input_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				inner_input_input_errors[this->inner_input_input_sizes[i_index]-1-s_index] = s_input_errors[this->inner_input_input_layer[i_index]+1].back();
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->inner_input_input_networks[i_index]->backprop_small_errors_with_no_weight_change(
				inner_input_input_errors,
				s_input_output_errors,
				state_output_errors,
				history->inner_input_input_network_histories[i_index]);
			for (int s_index = 0; s_index < (int)s_input_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]][s_index] += s_input_output_errors[s_index];
			}
			for (int s_index = 0; s_index < (int)state_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				state_errors[this->inner_input_input_layer[i_index]][s_index] += state_output_errors[s_index];
			}
		}
	}
}

void FinishedStep::update_activate(vector<vector<double>>& flat_vals,
								   vector<vector<double>>& s_input_vals,
								   vector<vector<double>>& state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->update_activate(flat_vals,
									 scope_input,
									 scope_output,
									 predicted_score,
									 scale_factor,
									 scope_history);
		history->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_vals.size()-1;
	}
	FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals,
										  score_network_history);
	history->score_network_history = score_network_history;
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::update_backprop(double& predicted_score,
								   double& next_predicted_score,
								   double target_val,
								   double& scale_factor,
								   FinishedStepHistory* history) {
	double misguess = abs(target_val - predicted_score);
	this->average_misguess = 0.999*this->average_misguess + 0.001*misguess;

	double predicted_score_error = target_val - predicted_score;

	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->score_network_history);
	next_predicted_score = predicted_score;
	predicted_score -= scale_factor*history->score_update;

	this->average_local_impact = 0.999*this->average_local_impact
		+ 0.001*abs(scale_factor*history->score_update);

	if (!this->is_inner_scope) {
		// do nothing
	} else {
		double ending_predicted_score = predicted_score;

		scale_factor *= this->scope_scale_mod;

		this->scope->update_backprop(predicted_score,
									 next_predicted_score,
									 target_val,
									 scale_factor,
									 history->scope_history);

		scale_factor /= this->scope_scale_mod;

		double starting_predicted_score = predicted_score;
		this->average_inner_scope_impact = 0.999*this->average_inner_scope_impact
			+ 0.001*abs(ending_predicted_score - starting_predicted_score);
	}
}

void FinishedStep::existing_update_activate(vector<vector<double>>& flat_vals,
											vector<vector<double>>& s_input_vals,
											vector<vector<double>>& state_vals,
											double& predicted_score,
											double& scale_factor,
											FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->existing_update_activate(flat_vals,
											  scope_input,
											  scope_output,
											  predicted_score,
											  scale_factor,
											  scope_history);
		history->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = (int)s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = (int)s_input_vals.size()-1;
	}
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals);
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::existing_update_backprop(double& predicted_score,
											double predicted_score_error,
											double& scale_factor,
											double& scale_factor_error,
											FinishedStepHistory* history) {
	scale_factor_error += history->score_update*predicted_score_error;

	predicted_score -= scale_factor*history->score_update;

	if (!this->is_inner_scope) {
		// do nothing
	} else {
		scale_factor *= this->scope_scale_mod;

		double scope_scale_factor_error = 0.0;
		this->scope->existing_update_backprop(predicted_score,
											  predicted_score_error,
											  scale_factor,
											  scope_scale_factor_error,
											  history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;
	}
}

void FinishedStep::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->is_inner_scope << endl;

	if (this->is_inner_scope) {
		output_file << this->scope->id << endl;
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(this->scope->id) + ".txt");
		this->scope->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->obs_size << endl;

	if (this->is_inner_scope) {
		output_file << this->inner_input_input_networks.size() << endl;
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			output_file << this->inner_input_input_layer[i_index] << endl;
			output_file << this->inner_input_input_sizes[i_index] << endl;

			ofstream inner_input_input_network_save_file;
			inner_input_input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_inner_input_input_" + to_string(i_index) + ".txt");
			this->inner_input_input_networks[i_index]->save(inner_input_input_network_save_file);
			inner_input_input_network_save_file.close();
		}

		ofstream inner_input_network_save_file;
		inner_input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_inner_input.txt");
		this->inner_input_network->save(inner_input_network_save_file);
		inner_input_network_save_file.close();

		output_file << this->scope_scale_mod << endl;
	}

	ofstream score_network_save_file;
	score_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_score.txt");
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	output_file << this->average_misguess << endl;
	output_file << this->average_inner_scope_impact << endl;
	output_file << this->average_local_impact << endl;

	output_file << this->compress_num_layers << endl;
	output_file << this->active_compress << endl;
	output_file << this->compress_new_size << endl;
	if (this->active_compress) {
		ofstream compress_network_save_file;
		compress_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_compress.txt");
		this->compress_network->save(compress_network_save_file);
		compress_network_save_file.close();
	}
	output_file << this->compress_original_size << endl;

	for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
		output_file << this->compressed_s_input_sizes[l_index] << endl;
		output_file << this->compressed_scope_sizes[l_index] << endl;
	}

	output_file << this->input_networks.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		output_file << this->input_layer[i_index] << endl;
		output_file << this->input_sizes[i_index] << endl;

		ofstream input_network_save_file;
		input_network_save_file.open("saves/nns/finished_step_" + to_string(this->id) + "_input_" + to_string(i_index) + ".txt");
		this->input_networks[i_index]->save(input_network_save_file);
		input_network_save_file.close();
	}
}

FinishedStepHistory::FinishedStepHistory(FinishedStep* finished_step) {
	this->finished_step = finished_step;

	this->inner_input_network_history = NULL;

	this->scope_history = NULL;

	this->score_network_history = NULL;

	this->compress_network_history = NULL;
}

FinishedStepHistory::~FinishedStepHistory() {
	for (int i_index = 0; i_index < (int)this->inner_input_input_network_histories.size(); i_index++) {
		delete this->inner_input_input_network_histories[i_index];
	}

	if (this->inner_input_network_history != NULL) {
		delete this->inner_input_network_history;
	}

	if (this->scope_history != NULL) {
		delete this->scope_history;
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}

	if (this->compress_network_history != NULL) {
		delete this->compress_network_history;
	}

	for (int i_index = 0; i_index < (int)this->input_network_histories.size(); i_index++) {
		delete this->input_network_histories[i_index];
	}
}
