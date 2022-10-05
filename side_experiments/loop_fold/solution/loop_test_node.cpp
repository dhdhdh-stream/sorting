#include "loop_test_node.h"

#include <iostream>

#include "test_node.h"

using namespace std;

LoopTestNode::LoopTestNode(vector<int> outer_scope_sizes,
						   Network* init_network,
						   vector<int> initial_inner_scope_sizes,
						   FoldLoopNetwork* original_loop,
						   FoldCombineNetwork* combine_network,
						   int obs_size) {
	this->obs_size = obs_size;

	this->outer_scope_sizes = outer_scope_sizes;
	this->init_network = init_network;
	this->combine_network = combine_network;

	this->curr_inner_scope_sizes = initial_inner_scope_sizes;
	this->curr_loop = new FoldLoopNetwork(original_loop);

	this->test_loop = NULL;

	vector<int> combined_scope_sizes = this->outer_scope_sizes;
	combined_scope_sizes.insert(combined_scope_sizes.end(),
		initial_inner_scope_sizes.begin(), initial_inner_scope_sizes.end());
	this->score_network = new ScoreNetwork(combined_scope_sizes,
										   this->obs_size);

	this->state_network = NULL;

	this->state = STATE_LEARN_SCORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
	this->best_sum_error = -1.0;

	this->new_scope_size = 0;
}

LoopTestNode::~LoopTestNode() {
	// do nothing

	// all networks will be taken or cleared by increment()
}

void LoopTestNode::loop_init(vector<vector<double>>& state_vals,
							 vector<double>& loop_state) {
	if (this->state == STATE_LEARN_SCORE) {
		// do nothing
	} else {
		vector<double> input;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
				input.push_back(state_vals[sc_index][st_index]);
			}
		}
		this->init_network->activate(input);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->init_network->output->acti_vals[s_index];
		}
	}
}

void LoopTestNode::activate(vector<vector<double>>& combined_state_vals,
							vector<bool>& combined_scopes_on,
							vector<double>& obs,
							vector<AbstractNetworkHistory*>& network_historys) {
	if (this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_CAN_COMPRESS_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE
			|| this->state == STATE_ADD_SCOPE_MEASURE) {
		this->score_network->activate(combined_state_vals,
									  obs);
		combined_state_vals[0][0] = this->score_network->output->acti_vals[0];

		if (this->state == STATE_JUST_SCORE_MEASURE) {
			combined_state_vals.erase(combined_state_vals.begin()+outer_scope_sizes.size(), combined_state_vals.end());
			// no need for scopes_on anymore
		} else {
			if (this->new_scope_size > 0) {
				combined_state_vals.push_back(vector<double>(this->new_scope_size));
				combined_scopes_on.push_back(true);
			}

			this->state_network->activate(combined_state_vals,
										  combined_scopes_on,
										  obs);
			for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
				combined_state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}

			if (this->state == STATE_COMPRESS_MEASURE
					&& this->test_compress_sizes > 1) {
				// activate all but last
				for (int c_index = 0; c_index < (int)this->compression_networks.size()-1; c_index++) {
					this->compression_networks[c_index]->activate(combined_state_vals,
																  combined_scopes_on);

					int sum_scope_sizes = 0;
					for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
						sum_scope_sizes += (int)combined_state_vals.back().size();
						combined_state_vals.pop_back();
						combined_scopes_on.pop_back();
					}
					combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
					combined_scopes_on.push_back(true);

					for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
						combined_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
					}
				}

				this->test_compression_network->activate(combined_state_vals,
														 combined_scopes_on);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
					sum_scope_sizes += (int)combined_state_vals.back().size();
					combined_state_vals.pop_back();
					combined_scopes_on.pop_back();
				}
				combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
				combined_scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
					combined_state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
				}
			} else if (this->state == STATE_CAN_COMPRESS_MEASURE
					|| this->state == STATE_COMPRESS_MEASURE) {
				for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
					this->compression_networks[c_index]->activate(combined_state_vals,
																  combined_scopes_on);

					int sum_scope_sizes = 0;
					for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
						sum_scope_sizes += (int)combined_state_vals.back().size();
						combined_state_vals.pop_back();
						combined_scopes_on.pop_back();
					}
					combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
					combined_scopes_on.push_back(true);

					for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
						combined_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
					}
				}

				this->test_compression_network->activate(combined_state_vals,
														 combined_scopes_on);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
					sum_scope_sizes += (int)combined_state_vals.back().size();
					combined_state_vals.pop_back();
					combined_scopes_on.pop_back();
				}
				combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
				combined_scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
					combined_state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
				}
			}
		}
	} else {
		if (this->state == STATE_LEARN_SCORE
				|| this->state == STATE_JUST_SCORE_TUNE
				|| this->state == STATE_LOCAL_SCOPE_TUNE
				|| this->state == STATE_COMPRESS_TUNE
				|| this->state == STATE_ADD_SCOPE_TUNE) {
			this->score_network->activate(combined_state_vals,
										  obs,
										  network_historys);
		} else {
			this->score_network->activate(combined_state_vals,
										  obs);
		}
		combined_state_vals[0][0] = this->score_network->output->acti_vals[0];

		if (this->state == STATE_LEARN_SCORE) {
			// do nothing
		} else if (this->state == STATE_JUST_SCORE_LEARN
				|| this->state == STATE_JUST_SCORE_TUNE) {
			combined_state_vals.erase(combined_state_vals.begin()+outer_scope_sizes.size(), combined_state_vals.end());
			// no need for scopes_on anymore
		} else {
			if (this->new_scope_size > 0) {
				combined_state_vals.push_back(vector<double>(this->new_scope_size));
				combined_scopes_on.push_back(true);
			}

			if (this->state == STATE_CAN_COMPRESS_LEARN
					|| this->state == STATE_COMPRESS_LEARN) {
				this->state_network->activate(combined_state_vals,
											  combined_scopes_on,
											  obs);
			} else {
				this->state_network->activate(combined_state_vals,
											  combined_scopes_on,
											  obs,
											  network_historys);
			}
			for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
				combined_state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}

			if (this->state == STATE_COMPRESS_TUNE) {
				for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
					this->compression_networks[c_index]->activate(combined_state_vals,
																  combined_scopes_on,
																  network_historys);

					int sum_scope_sizes = 0;
					for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
						sum_scope_sizes += (int)combined_state_vals.back().size();
						combined_state_vals.pop_back();
						combined_scopes_on.pop_back();
					}
					combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
					combined_scopes_on.push_back(true);

					for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
						combined_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
					}
				}
			} else if (this->state == STATE_COMPRESS_LEARN
					&& this->test_compress_sizes > 1) {
				// activate all but last
				for (int c_index = 0; c_index < (int)this->compression_networks.size()-1; c_index++) {
					this->compression_networks[c_index]->activate(combined_state_vals,
																  combined_scopes_on);

					int sum_scope_sizes = 0;
					for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
						sum_scope_sizes += (int)combined_state_vals.back().size();
						combined_state_vals.pop_back();
						combined_scopes_on.pop_back();
					}
					combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
					combined_scopes_on.push_back(true);

					for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
						combined_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
					}
				}

				this->test_compression_network->activate(combined_state_vals,
														 combined_scopes_on,
														 network_historys);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
					sum_scope_sizes += (int)combined_state_vals.back().size();
					combined_state_vals.pop_back();
					combined_scopes_on.pop_back();
				}
				combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
				combined_scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
					combined_state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
				}
			} else if (this->state == STATE_CAN_COMPRESS_LEARN
					|| this->state == STATE_COMPRESS_LEARN) {
				for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
					this->compression_networks[c_index]->activate(combined_state_vals,
																  combined_scopes_on);

					int sum_scope_sizes = 0;
					for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
						sum_scope_sizes += (int)combined_state_vals.back().size();
						combined_state_vals.pop_back();
						combined_scopes_on.pop_back();
					}
					combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
					combined_scopes_on.push_back(true);

					for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
						combined_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
					}
				}

				this->test_compression_network->activate(combined_state_vals,
														 combined_scopes_on,
														 network_historys);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
					sum_scope_sizes += (int)combined_state_vals.back().size();
					combined_state_vals.pop_back();
					combined_scopes_on.pop_back();
				}
				combined_state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
				combined_scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)combined_state_vals.back().size(); st_index++) {
					combined_state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
				}
			}
		}
	}
}

void LoopTestNode::loop_iter(vector<vector<double>>& loop_flat_vals,
							 vector<double>& loop_state,
							 vector<vector<double>>& combined_state_vals,
							 vector<AbstractNetworkHistory*>& network_historys) {
	if (this->state == STATE_LEARN_SCORE) {
		// do nothing
	} else if (this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_CAN_COMPRESS_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE
			|| this->state == STATE_ADD_SCOPE_MEASURE) {
		this->test_loop->loop_activate(loop_state,
									   loop_flat_vals,
									   combined_state_vals);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->test_loop->output->acti_vals[s_index];
		}
	} else if (this->state == STATE_JUST_SCORE_TUNE
			|| this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE
			|| this->state == STATE_ADD_SCOPE_TUNE) {
		this->curr_loop->loop_activate(loop_state,
									   loop_flat_vals,
									   combined_state_vals,
									   network_historys);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->curr_loop->output->acti_vals[s_index];
		}
	} else {
		this->test_loop->loop_activate(loop_state,
									   loop_flat_vals,
									   combined_state_vals,
									   network_historys);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->test_loop->output->acti_vals[s_index];
		}
	}
}

void LoopTestNode::process(vector<double>& loop_state,
						   vector<vector<double>>& post_loop_flat_vals,
						   vector<vector<double>>& outer_state_vals,
						   double target_val,
						   vector<Node*>& init_nodes,
						   vector<Node*>& loop_nodes,
						   vector<AbstractNetworkHistory*>& network_historys) {
	if (this->state == STATE_LEARN_SCORE) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		while (network_historys.size() > 0) {
			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			this->sum_error += abs(target_val - this->score_network->output->acti_vals[0]);

			if (this->state_iter <= 240000) {
				this->score_network->backprop_weights_with_no_error_signal(
					target_val,
					0.01);
			} else {
				this->score_network->backprop_weights_with_no_error_signal(
					target_val,
					0.002);
			}
		}
	} else if (this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_CAN_COMPRESS_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE
			|| this->state == STATE_ADD_SCOPE_MEASURE) {
		this->combine_network->loop_activate(loop_state,
											 post_loop_flat_vals,
											 outer_state_vals);

		this->sum_error += abs(target_val - this->combine_network->output->acti_vals[0]);
	} else if (this->state == STATE_JUST_SCORE_LEARN) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		this->combine_network->loop_activate(loop_state,
											 post_loop_flat_vals,
											 outer_state_vals);

		vector<double> combine_errors;
		combine_errors.push_back(target_val - this->combine_network->output->acti_vals[0]);
		sum_error += abs(combine_errors[0]);

		this->combine_network->backprop_loop_errors_with_no_weight_change(combine_errors);

		vector<double> loop_state_errors;
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state_errors.push_back(this->combine_network->loop_state_input->errors[s_index]);
			this->combine_network->loop_state_input->errors[s_index] = 0.0;
		}

		while (network_historys.size() > 0) {
			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			if (this->state_iter <= 40000) {
				this->test_loop->backprop_last_state(loop_state_errors, 0.01);
			} else {
				this->test_loop->backprop_last_state(loop_state_errors, 0.002);
			}
			for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
				loop_state_errors[s_index] = this->test_loop->loop_state_input->errors[s_index];
				this->test_loop->loop_state_input->errors[s_index] = 0.0;
			}
		}
	} else if (this->state == STATE_JUST_SCORE_TUNE) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		vector<vector<double>> state_errors;
		for (int sc_index = 0; sc_index < (int)this->outer_scope_sizes.size(); sc_index++) {
			state_errors.push_back(vector<double>(this->outer_scope_sizes[sc_index], 0.0));
		}

		while (network_historys.size() > 0) {
			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			this->sum_error += abs(target_val - this->score_network->output->acti_vals[0]);

			this->score_network->backprop(target_val, 0.002);

			for (int sc_index = (int)this->compressed_scope_sizes[0].size()-1; sc_index >= 0; sc_index--) {
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[0][sc_index], 0.0));
			}

			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->score_network->state_inputs[sc_index]->errors[st_index];
					this->score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}

			for (int n_index = (int)loop_nodes.size()-1; n_index >= 0; n_index--) {
				loop_nodes[n_index]->backprop(target_val,
											  state_errors,
											  network_historys);
			}

			// state_errors.size() should equal outer_scope_sizes.size()
		}

		for (int n_index = (int)init_nodes.size()-1; n_index >= 0; n_index--) {
			init_nodes[n_index]->backprop(target_val,
										  state_errors);
		}
	} else if (this->state == STATE_LOCAL_SCOPE_LEARN
			|| this->state == STATE_CAN_COMPRESS_LEARN
			|| this->state == STATE_COMPRESS_LEARN
			|| this->state == STATE_ADD_SCOPE_LEARN) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		this->combine_network->loop_activate(loop_state,
											 post_loop_flat_vals,
											 outer_state_vals);

		vector<double> combine_errors;
		combine_errors.push_back(target_val - this->combine_network->output->acti_vals[0]);
		sum_error += abs(combine_errors[0]);

		this->combine_network->backprop_loop_errors_with_no_weight_change(combine_errors);

		vector<double> loop_state_errors;
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state_errors.push_back(this->combine_network->loop_state_input->errors[s_index]);
			this->combine_network->loop_state_input->errors[s_index] = 0.0;
		}

		while (network_historys.size() > 0) {
			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			if (this->state_iter <= 40000) {
				this->test_loop->backprop_last_state(loop_state_errors, 0.01);
			} else {
				this->test_loop->backprop_last_state(loop_state_errors, 0.002);
			}
			for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
				loop_state_errors[s_index] = this->test_loop->loop_state_input->errors[s_index];
				this->test_loop->loop_state_input->errors[s_index] = 0.0;
			}
			vector<double> last_scope_state_errors(this->test_inner_scope_sizes.back());
			for (int st_index = 0; st_index < this->test_inner_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] = this->test_loop->state_inputs.back()->errors[st_index];
				this->test_loop->state_inputs.back()->errors[st_index] = 0.0;
			}

			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			if (this->state == STATE_CAN_COMPRESS_LEARN
					|| this->state == STATE_COMPRESS_LEARN) {
				if (this->state_iter <= 40000) {
					this->test_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.01);
				} else {
					this->test_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.002);
				}
			} else {
				if (this->state_iter <= 40000) {
					this->state_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.01);
				} else {
					this->state_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.002);
				}
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE
			|| this->state == STATE_ADD_SCOPE_TUNE) {
		if ((this->state_iter+1) % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		this->combine_network->loop_activate(loop_state,
											 post_loop_flat_vals,
											 outer_state_vals);

		vector<double> combine_errors;
		combine_errors.push_back(target_val - this->combine_network->output->acti_vals[0]);
		sum_error += abs(combine_errors[0]);

		this->combine_network->backprop_full_state(combine_errors, 0.002);
		vector<double> loop_state_errors;
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state_errors.push_back(this->combine_network->loop_state_input->errors[s_index]);
			this->combine_network->loop_state_input->errors[s_index] = 0.0;
		}
		vector<vector<double>> state_errors(this->outer_scope_sizes.size());
		// state_errors[0][0] doesn't matter
		for (int sc_index = 1; sc_index < (int)this->outer_scope_sizes.size(); sc_index++) {
			state_errors[sc_index].reserve(this->outer_scope_sizes[sc_index]);
			for (int st_index = 0; st_index < this->outer_scope_sizes[sc_index]; st_index++) {
				state_errors[sc_index].push_back(this->combine_network->state_inputs[sc_index]->errors[st_index]);
				this->combine_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}

		while (network_historys.size() > 0) {
			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			this->curr_loop->backprop_full_state(loop_state_errors, 0.002);
			for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
				loop_state_errors[s_index] = this->curr_loop->loop_state_input->errors[s_index];
				this->curr_loop->loop_state_input->errors[s_index] = 0.0;
			}
			for (int sc_index = 0; sc_index < (int)this->curr_inner_scope_sizes.size(); sc_index++) {
				state_errors.push_back(vector<double>(this->curr_inner_scope_sizes[sc_index], 0.0));
			}
			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] = this->curr_loop->state_inputs[sc_index]->errors[st_index];
					this->curr_loop->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}

			for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
				network_historys.back()->reset_weights();
				delete network_historys.back();
				network_historys.pop_back();

				this->compression_networks[c_index]->backprop(state_errors.back(),
															  0.002);

				state_errors.pop_back();
				for (int sc_index = this->compress_num_scopes[c_index]-1; sc_index >= 0; sc_index--) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[c_index][sc_index]));
				}

				// state_errors[0][0] doesn't matter
				for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
						state_errors[sc_index][st_index] += this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index];
						this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
					}
				}
			}

			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			this->state_network->backprop(state_errors.back(), 0.002);
			
			if (this->new_scope_size > 0) {
				state_errors.pop_back();
			}

			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size()-1; sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->state_network->state_inputs[sc_index]->errors[st_index];
					this->state_network->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] = this->state_network->state_inputs.back()->errors[st_index];
				this->state_network->state_inputs.back()->errors[st_index] = 0.0;
			}

			network_historys.back()->reset_weights();
			delete network_historys.back();
			network_historys.pop_back();

			this->score_network->backprop(target_val, 0.002);
			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->score_network->state_inputs[sc_index]->errors[st_index];
					this->score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}

			for (int n_index = (int)loop_nodes.size()-1; n_index >= 0; n_index--) {
				loop_nodes[n_index]->backprop(target_val,
											  state_errors,
											  network_historys);
			}

			// state_errors.size() should equal outer_scope_sizes.size()
		}

		this->init_network->backprop(loop_state_errors, 0.002);
		int input_index = 1;
		// state_errors[0][0] doesn't matter
		for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] = this->init_network->input->errors[input_index];
				this->init_network->input->errors[input_index] = 0.0;
				input_index++;
			}
		}

		for (int n_index = (int)init_nodes.size()-1; n_index >= 0; n_index--) {
			init_nodes[n_index]->backprop(target_val,
										  state_errors);
		}
	}

	increment();
}

void LoopTestNode::increment() {
	this->state_iter++;

	if (this->state == STATE_LEARN_SCORE) {
		if (this->state_iter > 320000) {
			if (this->state_iter%40000 == 0) {
				cout << this->state_iter << " " << this->sum_error << endl;
				if (this->best_sum_error == -1.0) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->tune_try = 0;
				} else {
					if (this->sum_error < this->best_sum_error) {
						this->best_sum_error = this->sum_error;
						this->sum_error = 0.0;
						this->tune_try = 0;
					} else if (this->tune_try < 2) {
						this->sum_error = 0.0;
						this->tune_try++;
					} else {
						cout << "ending STATE_LEARN_SCORE" << endl;
						cout << "starting STATE_JUST_SCORE_LEARN" << endl;

						this->test_inner_scope_sizes = vector<int>();

						this->test_loop = new FoldLoopNetwork(this->curr_loop);
						this->test_loop->inner_fold_index++;
						while (this->test_loop->state_inputs.size() > this->outer_scope_sizes.size()) {
							this->test_loop->pop_scope();
						}

						this->state = STATE_JUST_SCORE_LEARN;
						this->state_iter = 0;
						this->sum_error = 0.0;
					}
				}
			}
		} else {
			if (this->state_iter%40000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_JUST_SCORE_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_JUST_SCORE_LEARN" << endl;
			cout << "starting STATE_JUST_SCORE_MEASURE" << endl;

			this->state = STATE_JUST_SCORE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_JUST_SCORE_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < 1.2*this->combine_network->average_error) {
				cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_JUST_SCORE_TUNE" << endl;

				delete this->curr_loop;
				this->curr_loop = this->test_loop;
				this->test_loop = NULL;

				if (this->sum_error/10000 < this->combine_network->average_error) {
					this->combine_network->average_error = this->sum_error/10000;
				}

				this->compressed_scope_sizes.push_back(vector<int>());
				while (this->curr_inner_scope_sizes.size() > 0) {
					this->compressed_scope_sizes[0].push_back(this->curr_inner_scope_sizes.back());
					this->curr_inner_scope_sizes.pop_back();
				}

				this->just_score = true;
				this->update_existing_scope = false;

				this->state = STATE_JUST_SCORE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				delete this->test_loop;
				this->test_loop = NULL;

				this->just_score = false;

				if (this->curr_inner_scope_sizes.size() >= 1) {
					cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_LOCAL_SCOPE_LEARN" << endl;

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					this->test_loop->inner_fold_index++;
					this->test_loop->reset_last();

					this->test_inner_scope_sizes = this->curr_inner_scope_sizes;

					vector<int> combined_scope_sizes = this->outer_scope_sizes;
					combined_scope_sizes.insert(combined_scope_sizes.end(),
						this->test_inner_scope_sizes.begin(), this->test_inner_scope_sizes.end());
					this->state_network = new StateNetwork(combined_scope_sizes,
														   this->obs_size,
														   combined_scope_sizes.back());

					this->state = STATE_LOCAL_SCOPE_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_ADD_SCOPE_LEARN" << endl;

					this->update_existing_scope = false;

					this->new_scope_size = 1;

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					this->test_loop->inner_fold_index++;
					this->test_loop->add_scope(this->new_scope_size);

					this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
					this->test_inner_scope_sizes.push_back(this->new_scope_size);

					vector<int> combined_scope_sizes = this->outer_scope_sizes;
					// use curr_scope_sizes for construction
					combined_scope_sizes.insert(combined_scope_sizes.end(),
						this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
					this->state_network = new StateNetwork(combined_scope_sizes,
														   this->obs_size,
														   this->new_scope_size);

					this->state = STATE_ADD_SCOPE_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				}
			}
		}
	} else if (this->state == STATE_JUST_SCORE_TUNE) {
		if (this->state_iter >= 40000) {
			if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->state_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try++;
				} else {
					cout << "ending STATE_JUST_SCORE_TUNE" << endl;
					cout << "DONE" << endl;

					this->state = STATE_DONE;
				}
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_LOCAL_SCOPE_LEARN" << endl;
			cout << "starting STATE_LOCAL_SCOPE_MEASURE" << endl;

			this->state = STATE_LOCAL_SCOPE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_LOCAL_SCOPE_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < 1.2*this->combine_network->average_error) {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_LOCAL_SCOPE_TUNE" << endl;

				this->update_existing_scope = true;

				delete this->curr_loop;
				this->curr_loop = this->test_loop;
				this->test_loop = NULL;

				if (this->sum_error/10000 < this->combine_network->average_error) {
					this->combine_network->average_error = this->sum_error/10000;
				}

				this->curr_inner_scope_sizes = this->test_inner_scope_sizes;

				this->state = STATE_LOCAL_SCOPE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_ADD_SCOPE_LEARN" << endl;

				this->update_existing_scope = false;

				this->new_scope_size = 1;

				delete this->test_loop;
				this->test_loop = new FoldLoopNetwork(this->curr_loop);
				this->test_loop->outer_fold_index++;
				this->test_loop->add_scope(this->new_scope_size);

				this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
				this->test_inner_scope_sizes.push_back(this->new_scope_size);

				delete this->state_network;
				vector<int> combined_scope_sizes = this->outer_scope_sizes;
				// use curr_scope_sizes for construction
				combined_scope_sizes.insert(combined_scope_sizes.end(),
					this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
				this->state_network = new StateNetwork(combined_scope_sizes,
													   this->obs_size,
													   this->new_scope_size);

				this->state = STATE_ADD_SCOPE_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE) {
		if (this->state_iter >= 40000) {
			if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->state_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try++;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < (int)this->curr_inner_scope_sizes.size(); sc_index++) {
						sum_scope_sizes += this->curr_inner_scope_sizes[sc_index];
					}
					if (sum_scope_sizes >= 2) {
						cout << "ending STATE_LOCAL_SCOPE_TUNE" << endl;
						cout << "starting STATE_CAN_COMPRESS_LEARN" << endl;

						this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
						this->test_compress_num_scopes = (int)this->test_inner_scope_sizes.size();
						this->test_compressed_scope_sizes = vector<int>(this->test_inner_scope_sizes.size());
						for (int sc_index = (int)this->test_inner_scope_sizes.size()-1; sc_index >= 0; sc_index--) {
							this->test_compressed_scope_sizes[sc_index] = this->test_inner_scope_sizes.back();
							this->test_inner_scope_sizes.pop_back();
						}
						this->test_compress_sizes = 1;
						this->test_inner_scope_sizes.push_back(sum_scope_sizes-1);

						vector<int> combined_scope_sizes = this->outer_scope_sizes;
						// use curr_scope_sizes for construction
						combined_scope_sizes.insert(combined_scope_sizes.end(),
							this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
						this->test_compression_network = new CompressionNetwork(combined_scope_sizes,
																				sum_scope_sizes-1);

						this->test_loop = new FoldLoopNetwork(this->curr_loop);
						while (this->test_loop->state_inputs.size() > this->outer_scope_sizes.size()) {
							this->test_loop->pop_scope();
						}
						this->test_loop->add_scope(sum_scope_sizes-1);

						this->state = STATE_CAN_COMPRESS_LEARN;
						this->state_iter = 0;
						this->sum_error = 0.0;
					} else {
						cout << "ending STATE_LOCAL_SCOPE_TUNE" << endl;
						cout << "DONE" << endl;

						this->state = STATE_DONE;
					}
				}
			}
		}
	} else if (this->state == STATE_CAN_COMPRESS_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_CAN_COMPRESS_LEARN" << endl;
			cout << "starting STATE_CAN_COMPRESS_MEASURE" << endl;

			this->state = STATE_CAN_COMPRESS_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_CAN_COMPRESS_MEASURE) {
		if (this->state_iter >= 10000) {
			delete this->test_compression_network;

			delete this->test_loop;
			this->test_loop = NULL;

			if (this->sum_error/10000 < 1.2*this->combine_network->average_error) {
				cout << "ending STATE_COMPRESS_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_COMPRESS_LEARN" << endl;

				this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
				if (this->test_inner_scope_sizes.back() == 1) {
					this->test_compress_num_scopes = 2;
					this->test_compress_sizes = 1;
					this->test_compressed_scope_sizes = vector<int>(2);
					int sum_scope_sizes = 0;
					this->test_compressed_scope_sizes[1] = this->test_inner_scope_sizes.back();
					sum_scope_sizes += this->test_inner_scope_sizes.back();
					this->test_inner_scope_sizes.pop_back();
					this->test_compressed_scope_sizes[0] = this->test_inner_scope_sizes.back();
					sum_scope_sizes += this->test_inner_scope_sizes.back();
					this->test_inner_scope_sizes.pop_back();
					this->test_inner_scope_sizes.push_back(sum_scope_sizes-1);

					vector<int> combined_scope_sizes = this->outer_scope_sizes;
					// use curr_scope_sizes for construction
					combined_scope_sizes.insert(combined_scope_sizes.end(),
						this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
					this->test_compression_network = new CompressionNetwork(combined_scope_sizes,
																			sum_scope_sizes-1);

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					this->test_loop->pop_scope();
					this->test_loop->pop_scope();
					this->test_loop->add_scope(sum_scope_sizes-1);
				} else {
					this->test_compress_num_scopes = 1;
					this->test_compress_sizes = 1;
					this->test_compressed_scope_sizes = vector<int>{this->test_inner_scope_sizes.back()};
					int sum_scope_sizes = this->test_inner_scope_sizes.back();
					this->test_inner_scope_sizes.pop_back();
					this->test_inner_scope_sizes.push_back(sum_scope_sizes-1);

					vector<int> combined_scope_sizes = this->outer_scope_sizes;
					// use curr_scope_sizes for construction
					combined_scope_sizes.insert(combined_scope_sizes.end(),
						this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
					this->test_compression_network = new CompressionNetwork(combined_scope_sizes,
																			sum_scope_sizes-1);

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					this->test_loop->pop_scope();
					this->test_loop->add_scope(sum_scope_sizes-1);
				}

				this->state = STATE_COMPRESS_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_CAN_COMPRESS_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "DONE" << endl;

				this->state = STATE_DONE;
			}
		}
	} else if (this->state == STATE_COMPRESS_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_COMPRESS_LEARN" << endl;
			cout << "starting STATE_COMPRESS_MEASURE" << endl;

			this->state = STATE_COMPRESS_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < this->combine_network->average_error
					|| this->test_inner_scope_sizes.size() == 1) {
				if (this->test_compress_sizes > 1) {
					this->compress_num_scopes.pop_back();
					this->compress_num_scopes.push_back(this->test_compress_num_scopes);
					this->compress_sizes.pop_back();
					this->compress_sizes.push_back(this->test_compress_sizes);
					delete this->compression_networks.back();
					this->compression_networks.pop_back();
					this->compression_networks.push_back(this->test_compression_network);
					this->test_compression_network = NULL;
					this->compressed_scope_sizes.pop_back();
					this->compressed_scope_sizes.push_back(this->test_compressed_scope_sizes);
				} else {
					this->compress_num_scopes.push_back(this->test_compress_num_scopes);
					this->compress_sizes.push_back(this->test_compress_sizes);
					this->compression_networks.push_back(this->test_compression_network);
					this->test_compression_network = NULL;
					this->compressed_scope_sizes.push_back(this->test_compressed_scope_sizes);
				}

				delete this->curr_loop;
				this->curr_loop = this->test_loop;
				this->test_loop = NULL;

				if (this->sum_error/10000 < this->combine_network->average_error) {
					this->combine_network->average_error = this->sum_error/10000;
				}

				this->curr_inner_scope_sizes = this->test_inner_scope_sizes;

				int sum_scope_sizes = 0;
				for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.back().size(); sc_index++) {
					sum_scope_sizes += this->compressed_scope_sizes.back()[sc_index];
				}

				if (sum_scope_sizes - this->compress_sizes.back() == 1) {
					cout << "ending STATE_COMPRESS_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_TUNE" << endl;

					this->state = STATE_COMPRESS_TUNE;
					this->state_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					cout << "ending STATE_COMPRESS_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_LEARN" << endl;

					this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
					this->test_compress_num_scopes = this->compress_num_scopes.back();
					this->test_compress_sizes = this->compress_sizes.back()+1;
					this->test_compressed_scope_sizes = this->compressed_scope_sizes.back();
					this->test_inner_scope_sizes.pop_back();
					this->test_inner_scope_sizes.push_back(sum_scope_sizes-this->test_compress_sizes);

					this->test_compression_network = new CompressionNetwork(
						this->compression_networks.back()->scope_sizes,
						sum_scope_sizes-this->test_compress_sizes);

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					this->test_loop->pop_scope();
					this->test_loop->add_scope(sum_scope_sizes-this->test_compress_sizes);

					this->state = STATE_COMPRESS_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				delete this->test_compression_network;

				delete this->test_loop;
				this->test_loop = NULL;

				if (this->test_compress_sizes == 1) {
					cout << "ending STATE_COMPRESS_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_LEARN" << endl;

					this->test_compress_num_scopes++;
					this->test_inner_scope_sizes.pop_back();
					this->test_compressed_scope_sizes.push_back(this->test_inner_scope_sizes.back());

					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->test_compress_num_scopes; sc_index++) {
						sum_scope_sizes += this->curr_inner_scope_sizes[this->curr_inner_scope_sizes.size()-1-sc_index];
					}

					this->test_inner_scope_sizes.pop_back();
					this->test_inner_scope_sizes.push_back(sum_scope_sizes-1);

					vector<int> combined_scope_sizes = this->outer_scope_sizes;
					// use curr_scope_sizes for construction
					combined_scope_sizes.insert(combined_scope_sizes.end(),
						this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
					this->test_compression_network = new CompressionNetwork(combined_scope_sizes,
																			sum_scope_sizes-1);

					this->test_loop = new FoldLoopNetwork(this->curr_loop);
					for (int sc_index = 0; sc_index < this->test_compress_num_scopes; sc_index++) {
						this->test_loop->pop_scope();
					}
					this->test_loop->add_scope(sum_scope_sizes-1);

					this->state = STATE_COMPRESS_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_COMPRESS_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_TUNE" << endl;

					this->state = STATE_COMPRESS_TUNE;
					this->state_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_TUNE) {
		if (this->state_iter >= 40000) {
			if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->state_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try++;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < (int)this->curr_inner_scope_sizes.size(); sc_index++) {
						sum_scope_sizes += this->curr_inner_scope_sizes[sc_index];
					}
					if (sum_scope_sizes >= 2) {
						cout << "ending STATE_COMPRESS_TUNE" << endl;
						cout << "starting STATE_CAN_COMPRESS_LEARN" << endl;

						this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
						this->test_compress_num_scopes = (int)this->test_inner_scope_sizes.size();
						this->test_compressed_scope_sizes = vector<int>(this->test_inner_scope_sizes.size());
						for (int sc_index = (int)this->test_inner_scope_sizes.size()-1; sc_index >= 0; sc_index--) {
							this->test_compressed_scope_sizes[sc_index] = this->test_inner_scope_sizes.back();
							this->test_inner_scope_sizes.pop_back();
						}
						this->test_compress_sizes = 1;
						this->test_inner_scope_sizes.push_back(sum_scope_sizes-1);

						vector<int> combined_scope_sizes = this->outer_scope_sizes;
						// use curr_scope_sizes for construction
						combined_scope_sizes.insert(combined_scope_sizes.end(),
							this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
						this->test_compression_network = new CompressionNetwork(combined_scope_sizes,
																				sum_scope_sizes-1);

						this->test_loop = new FoldLoopNetwork(this->curr_loop);
						while (this->test_loop->state_inputs.size() > this->outer_scope_sizes.size()) {
							this->test_loop->pop_scope();
						}
						this->test_loop->add_scope(sum_scope_sizes-1);

						this->state = STATE_CAN_COMPRESS_LEARN;
						this->state_iter = 0;
						this->sum_error = 0.0;
					} else {
						cout << "ending STATE_COMPRESS_TUNE" << endl;
						cout << "DONE" << endl;

						this->state = STATE_DONE;
					}
				}
			}
		}
	} else if (this->state == STATE_ADD_SCOPE_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_ADD_SCOPE_LEARN" << endl;
			cout << "starting STATE_ADD_SCOPE_MEASURE" << endl;

			this->state = STATE_ADD_SCOPE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_ADD_SCOPE_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < 1.2*this->combine_network->average_error) {
				cout << "ending STATE_ADD_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_ADD_SCOPE_TUNE" << endl;

				delete this->curr_loop;
				this->curr_loop = this->test_loop;
				this->test_loop = NULL;

				if (this->sum_error/10000 < this->combine_network->average_error) {
					this->combine_network->average_error = this->sum_error/10000;
				}

				this->curr_inner_scope_sizes = this->test_inner_scope_sizes;

				this->state = STATE_ADD_SCOPE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_ADD_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_ADD_SCOPE_LEARN" << endl;

				this->new_scope_size++;

				delete this->test_loop;
				this->test_loop = new FoldLoopNetwork(this->curr_loop);
				this->test_loop->outer_fold_index++;
				this->test_loop->add_scope(this->new_scope_size);

				this->test_inner_scope_sizes = this->curr_inner_scope_sizes;
				this->test_inner_scope_sizes.push_back(this->new_scope_size);

				delete this->state_network;
				vector<int> combined_scope_sizes = this->outer_scope_sizes;
				// use curr_scope_sizes for construction
				combined_scope_sizes.insert(combined_scope_sizes.end(),
					this->curr_inner_scope_sizes.begin(), this->curr_inner_scope_sizes.end());
				this->state_network = new StateNetwork(combined_scope_sizes,
													   this->obs_size,
													   this->new_scope_size);

				this->state = STATE_ADD_SCOPE_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_ADD_SCOPE_TUNE) {
		if (this->state_iter >= 40000) {
			if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->state_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->state_iter = 0;
					this->tune_try++;
				} else {
					cout << "ending STATE_ADD_SCOPE_TUNE" << endl;
					cout << "DONE" << endl;

					this->state = STATE_DONE;
				}
			}
		}
	}
}
