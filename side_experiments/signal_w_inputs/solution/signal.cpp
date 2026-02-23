#include "signal.h"

#include "constants.h"
#include "globals.h"
#include "layer.h"
#include "signal_helpers.h"
#include "signal_node.h"

using namespace std;

Signal::Signal() {
	this->output_constant = 0.0;
	this->output_constant_update = 0.0;

	this->epoch_iter = 0;
	this->average_max_update = 0.0;

	this->existing_history_index = 0;
	this->explore_history_index = 0;
}

Signal::~Signal() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

double Signal::activate(ScopeHistory* scope_history,
						vector<int>& explore_index) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		vector<double> input_vals(this->nodes[n_index]->inputs.size());
		vector<bool> input_is_on(this->nodes[n_index]->inputs.size());
		for (int i_index = 0; i_index < (int)this->nodes[n_index]->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->nodes[n_index]->inputs[i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->nodes[n_index]->activate(input_vals,
									   input_is_on);
	}

	double sum_vals = this->output_constant;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		sum_vals += this->output_weights[n_index] * this->nodes[n_index]->output->acti_vals[0];
	}

	return sum_vals;
}

double Signal::activate_helper(bool is_existing,
							   int index) {
	if (is_existing) {
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->activate(this->existing_input_val_histories[index][n_index],
										   this->existing_input_is_on_histories[index][n_index]);
		}
	} else {
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->activate(this->explore_input_val_histories[index][n_index],
										   this->explore_input_is_on_histories[index][n_index]);
		}
	}

	double sum_vals = this->output_constant;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		sum_vals += this->output_weights[n_index] * this->nodes[n_index]->output->acti_vals[0];
	}

	return sum_vals;
}

void Signal::backprop_helper(bool is_existing,
							 int index) {
	double target_val;
	if (is_existing) {
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->activate(this->existing_input_val_histories[index][n_index],
										   this->existing_input_is_on_histories[index][n_index]);
		}

		target_val = this->existing_target_val_histories[index];
	} else {
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->activate(this->explore_input_val_histories[index][n_index],
										   this->explore_input_is_on_histories[index][n_index]);
		}

		target_val = this->explore_target_val_histories[index];
	}

	double sum_vals = this->output_constant;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		sum_vals += this->output_weights[n_index] * this->nodes[n_index]->output->acti_vals[0];
	}

	double error = target_val - sum_vals;

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->output->errors[0] += error * this->output_weights[n_index];

		this->output_weight_updates[n_index] += error * this->nodes[n_index]->output->acti_vals[0];
	}
	this->output_constant_update += error;

	for (int n_index = (int)this->nodes.size()-1; n_index >= 0; n_index--) {
		this->nodes[n_index]->backprop();
	}

	this->epoch_iter++;
	if (this->epoch_iter == NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			double update_size = abs(this->output_weight_updates[n_index]);
			if (update_size > max_update) {
				max_update = update_size;
			}
		}
		{
			double update_size = abs(this->output_constant_update);
			if (update_size > max_update) {
				max_update = update_size;
			}
		}

		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->get_max_update(max_update);
		}

		this->average_max_update = 0.999*this->average_max_update + 0.001*max_update;
		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_update;
			if (learning_rate * max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
				this->nodes[n_index]->update_weights(learning_rate);
			}

			for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
				double update = learning_rate * this->output_weight_updates[n_index];
				this->output_weight_updates[n_index] = 0.0;
				this->output_weights[n_index] += update;
			}
			{
				double update = learning_rate * this->output_constant_update;
				this->output_constant_update = 0.0;
				this->output_constant += update;
			}

			this->epoch_iter = 0;
		}
	}
}

void Signal::clean_inputs(Scope* scope,
						  int node_id) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		for (int i_index = (int)this->nodes[n_index]->inputs.size()-1; i_index >= 0; i_index--) {
			bool is_match = false;
			for (int l_index = 0; l_index < (int)this->nodes[n_index]->inputs[i_index].scope_context.size(); l_index++) {
				if (this->nodes[n_index]->inputs[i_index].scope_context[l_index] == scope
						&& this->nodes[n_index]->inputs[i_index].node_context[l_index] == node_id) {
					is_match = true;
					break;
				}
			}

			if (is_match) {
				this->nodes[n_index]->remove_input(i_index);
				for (int h_index = 0; h_index < (int)this->existing_input_val_histories.size(); h_index++) {
					this->existing_input_val_histories[h_index][n_index].erase(
						this->existing_input_val_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->existing_input_is_on_histories.size(); h_index++) {
					this->existing_input_is_on_histories[h_index][n_index].erase(
						this->existing_input_is_on_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->explore_input_val_histories.size(); h_index++) {
					this->explore_input_val_histories[h_index][n_index].erase(
						this->explore_input_val_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->explore_input_is_on_histories.size(); h_index++) {
					this->explore_input_is_on_histories[h_index][n_index].erase(
						this->explore_input_is_on_histories[h_index][n_index].begin() + i_index);
				}
			}
		}
	}

	for (int p_index = 0; p_index < (int)this->potential_inputs.size(); p_index++) {
		for (int i_index = (int)this->potential_inputs[p_index].size()-1; i_index >= 0; i_index--) {
			bool is_match = false;
			for (int l_index = 0; l_index < (int)this->potential_inputs[p_index][i_index].scope_context.size(); l_index++) {
				if (this->potential_inputs[p_index][i_index].scope_context[l_index] == scope
						&& this->potential_inputs[p_index][i_index].node_context[l_index] == node_id) {
					is_match = true;
					break;
				}
			}

			if (is_match) {
				this->potential_inputs[p_index].erase(this->potential_inputs[p_index].begin() + i_index);
				for (int h_index = 0; h_index < (int)this->potential_existing_input_val_histories.size(); h_index++) {
					this->potential_existing_input_val_histories[h_index][p_index].erase(
						this->potential_existing_input_val_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_existing_input_is_on_histories.size(); h_index++) {
					this->potential_existing_input_is_on_histories[h_index][p_index].erase(
						this->potential_existing_input_is_on_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_explore_input_val_histories.size(); h_index++) {
					this->potential_explore_input_val_histories[h_index][p_index].erase(
						this->potential_explore_input_val_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_explore_input_is_on_histories.size(); h_index++) {
					this->potential_explore_input_is_on_histories[h_index][p_index].erase(
						this->potential_explore_input_is_on_histories[h_index][p_index].begin() + i_index);
				}
			}
		}
	}
}

void Signal::clean_inputs(Scope* scope) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		for (int i_index = (int)this->nodes[n_index]->inputs.size()-1; i_index >= 0; i_index--) {
			bool is_match = false;
			for (int l_index = 0; l_index < (int)this->nodes[n_index]->inputs[i_index].scope_context.size(); l_index++) {
				if (this->nodes[n_index]->inputs[i_index].scope_context[l_index] == scope) {
					is_match = true;
					break;
				}
			}

			if (is_match) {
				this->nodes[n_index]->remove_input(i_index);
				for (int h_index = 0; h_index < (int)this->existing_input_val_histories.size(); h_index++) {
					this->existing_input_val_histories[h_index][n_index].erase(
						this->existing_input_val_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->existing_input_is_on_histories.size(); h_index++) {
					this->existing_input_is_on_histories[h_index][n_index].erase(
						this->existing_input_is_on_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->explore_input_val_histories.size(); h_index++) {
					this->explore_input_val_histories[h_index][n_index].erase(
						this->explore_input_val_histories[h_index][n_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->explore_input_is_on_histories.size(); h_index++) {
					this->explore_input_is_on_histories[h_index][n_index].erase(
						this->explore_input_is_on_histories[h_index][n_index].begin() + i_index);
				}
			}
		}
	}

	for (int p_index = 0; p_index < (int)this->potential_inputs.size(); p_index++) {
		for (int i_index = (int)this->potential_inputs[p_index].size()-1; i_index >= 0; i_index--) {
			bool is_match = false;
			for (int l_index = 0; l_index < (int)this->potential_inputs[p_index][i_index].scope_context.size(); l_index++) {
				if (this->potential_inputs[p_index][i_index].scope_context[l_index] == scope) {
					is_match = true;
					break;
				}
			}

			if (is_match) {
				this->potential_inputs[p_index].erase(this->potential_inputs[p_index].begin() + i_index);
				for (int h_index = 0; h_index < (int)this->potential_existing_input_val_histories.size(); h_index++) {
					this->potential_existing_input_val_histories[h_index][p_index].erase(
						this->potential_existing_input_val_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_existing_input_is_on_histories.size(); h_index++) {
					this->potential_existing_input_is_on_histories[h_index][p_index].erase(
						this->potential_existing_input_is_on_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_explore_input_val_histories.size(); h_index++) {
					this->potential_explore_input_val_histories[h_index][p_index].erase(
						this->potential_explore_input_val_histories[h_index][p_index].begin() + i_index);
				}
				for (int h_index = 0; h_index < (int)this->potential_explore_input_is_on_histories.size(); h_index++) {
					this->potential_explore_input_is_on_histories[h_index][p_index].erase(
						this->potential_explore_input_is_on_histories[h_index][p_index].begin() + i_index);
				}
			}
		}
	}
}

void Signal::replace_obs_node(Scope* scope,
							  int original_node_id,
							  int new_node_id) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		for (int i_index = 0; i_index < (int)this->nodes[n_index]->inputs.size(); i_index++) {
			if (this->nodes[n_index]->inputs[i_index].scope_context.back() == scope
					&& this->nodes[n_index]->inputs[i_index].node_context.back() == original_node_id) {
				this->nodes[n_index]->inputs[i_index].node_context.back() = new_node_id;
			}
		}
	}

	for (int p_index = 0; p_index < (int)this->potential_inputs.size(); p_index++) {
		for (int i_index = 0; i_index < (int)this->potential_inputs[p_index].size(); i_index++) {
			if (this->potential_inputs[p_index][i_index].scope_context.back() == scope
					&& this->potential_inputs[p_index][i_index].node_context.back() == original_node_id) {
				this->potential_inputs[p_index][i_index].node_context.back() = new_node_id;
			}
		}
	}
}

void Signal::save(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(output_file);
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->output_weights[n_index] << endl;
	}
	output_file << this->output_constant << endl;
}

void Signal::load(ifstream& input_file,
				  Solution* parent_solution) {
	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		SignalNode* signal_node = new SignalNode(input_file,
												 parent_solution);
		this->nodes.push_back(signal_node);
	}

	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->output_weights.push_back(stod(weight_line));
	}

	string output_constant_line;
	getline(input_file, output_constant_line);
	this->output_constant = stod(output_constant_line);

	for (int n_index = 0; n_index < num_nodes; n_index++) {
		this->output_weight_updates.push_back(0.0);
	}
}

void Signal::copy_from(Signal* original,
					   Solution* parent_solution) {
	for (int n_index = 0; n_index < (int)original->nodes.size(); n_index++) {
		this->nodes.push_back(new SignalNode(original->nodes[n_index],
											 parent_solution));
	}

	this->output_weights = original->output_weights;
	this->output_constant = original->output_constant;
	this->output_weight_updates = original->output_weight_updates;
	this->output_constant_update = original->output_constant_update;

	this->epoch_iter = original->epoch_iter;
	this->average_max_update = original->average_max_update;

	this->existing_input_val_histories = original->existing_input_val_histories;
	this->existing_input_is_on_histories = original->existing_input_is_on_histories;
	this->existing_target_val_histories = original->existing_target_val_histories;
	this->existing_history_index = original->existing_history_index;
	this->explore_input_val_histories = original->explore_input_val_histories;
	this->explore_input_is_on_histories = original->explore_input_is_on_histories;
	this->explore_target_val_histories = original->explore_target_val_histories;
	this->explore_history_index = original->explore_history_index;

	for (int p_index = 0; p_index < (int)original->potential_inputs.size(); p_index++) {
		vector<SignalInput> inputs;
		for (int i_index = 0; i_index < (int)original->potential_inputs[p_index].size(); i_index++) {
			inputs.push_back(SignalInput(original->potential_inputs[p_index][i_index],
										 parent_solution));
		}
		this->potential_inputs.push_back(inputs);
	}
	this->potential_existing_input_val_histories = original->potential_existing_input_val_histories;
	this->potential_existing_input_is_on_histories = original->potential_existing_input_is_on_histories;
	this->potential_existing_count = original->potential_existing_count;
	this->potential_explore_input_val_histories = original->potential_explore_input_val_histories;
	this->potential_explore_input_is_on_histories = original->potential_explore_input_is_on_histories;
	this->potential_explore_count = original->potential_explore_count;
}
