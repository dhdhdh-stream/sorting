#include "test_node.h"

#include <iostream>

using namespace std;

TestNode::TestNode(vector<int> initial_scope_sizes,
				   FoldNetwork* original_fold,
				   int obs_size) {
	this->obs_size = obs_size;

	this->curr_scope_sizes = initial_scope_sizes;
	this->curr_fold = new FoldNetwork(original_fold);

	this->test_fold = NULL;

	this->score_network = new ScoreNetwork(initial_scope_sizes,
										   this->obs_size);

	this->state_network = NULL;

	this->state = STATE_LEARN_SCORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
	this->best_sum_error = -1.0;

	this->new_scope_size = 0;
}

TestNode::~TestNode() {
	// do nothing

	// all networks will be taken or cleared by increment()
}

void TestNode::activate(vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						vector<double>& obs) {
	this->score_network->activate(state_vals,
								  obs);
	state_vals[0][0] = this->score_network->output->acti_vals[0];

	if (this->state == STATE_LEARN_SCORE) {
		// do nothing
	} else if (this->state == STATE_JUST_SCORE_LEARN
			|| this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_JUST_SCORE_TUNE) {
		state_vals.erase(state_vals.begin()+1, state_vals.end());
		// no need for scopes_on anymore
	} else {
		if (this->new_scope_size > 0) {
			state_vals.push_back(vector<double>(this->new_scope_size));
			scopes_on.push_back(true);
		}

		this->state_network->activate(state_vals,
									  scopes_on,
									  obs);
		for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
			state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
		}

		if (this->state == STATE_COMPRESS_TUNE) {
			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				this->compression_networks[c_index]->activate(state_vals,
															  scopes_on);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
					sum_scope_sizes += (int)state_vals.back().size();
					state_vals.pop_back();
					scopes_on.pop_back();
				}
				state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
				scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
				}
			}
		} else if ((this->state == STATE_COMPRESS_LEARN
				|| this->state == STATE_COMPRESS_MEASURE)
				&& this->test_compress_sizes > 1) {
			// activate all but last
			for (int c_index = 0; c_index < (int)this->compression_networks.size()-1; c_index++) {
				this->compression_networks[c_index]->activate(state_vals,
															  scopes_on);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
					sum_scope_sizes += (int)state_vals.back().size();
					state_vals.pop_back();
					scopes_on.pop_back();
				}
				state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
				scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
				}
			}

			this->test_compression_network->activate(state_vals,
													 scopes_on);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
			}
		} else if (this->state == STATE_CAN_COMPRESS_LEARN
				|| this->state == STATE_CAN_COMPRESS_MEASURE
				|| this->state == STATE_COMPRESS_LEARN
				|| this->state == STATE_COMPRESS_MEASURE) {
			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				this->compression_networks[c_index]->activate(state_vals,
															  scopes_on);

				int sum_scope_sizes = 0;
				for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
					sum_scope_sizes += (int)state_vals.back().size();
					state_vals.pop_back();
					scopes_on.pop_back();
				}
				state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
				scopes_on.push_back(true);

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
				}
			}

			this->test_compression_network->activate(state_vals,
													 scopes_on);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
			}
		}
	}
}

void TestNode::process(vector<vector<double>>& flat_inputs,
					   vector<vector<double>>& state_vals,
					   double target_val,
					   vector<Node*>& nodes) {
	if (this->state == STATE_LEARN_SCORE) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

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
	} else if (this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_CAN_COMPRESS_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE
			|| this->state == STATE_ADD_SCOPE_MEASURE) {
		this->test_fold->activate(flat_inputs,
								  state_vals);

		this->sum_error += abs(target_val - this->test_fold->output->acti_vals[0]);
	} else if (this->state == STATE_JUST_SCORE_LEARN) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		this->test_fold->activate(flat_inputs,
								  state_vals);

		vector<double> errors;
		errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
		this->sum_error += abs(errors[0]);

		if (this->state_iter <= 240000) {
			this->test_fold->backprop_last_state(errors, 0.01);
		} else {
			this->test_fold->backprop_last_state(errors, 0.002);
		}
	} else if (this->state == STATE_JUST_SCORE_TUNE) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		this->sum_error += abs(target_val - this->score_network->output->acti_vals[0]);

		this->score_network->backprop(target_val, 0.002);

		vector<vector<double>> state_errors;
		state_errors.push_back(vector<double>{0.0});

		for (int sc_index = (int)this->compressed_scope_sizes[0].size()-1; sc_index >= 0; sc_index--) {
			state_errors.push_back(vector<double>(this->compressed_scope_sizes[0][sc_index]));
		}

		// state_errors[0][0] doesn't matter
		for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] = this->score_network->state_inputs[sc_index]->errors[st_index];
				this->score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}

		for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
			nodes[n_index]->backprop(target_val,
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

		this->test_fold->activate(flat_inputs,
								  state_vals);

		vector<double> errors;
		errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
		this->sum_error += abs(errors[0]);

		if (this->state_iter <= 240000) {
			this->test_fold->backprop_last_state(errors, 0.01);

			vector<double> last_scope_state_errors(this->test_scope_sizes.back());
			for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
				this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
			}

			if (this->state == STATE_CAN_COMPRESS_LEARN
					|| this->state == STATE_COMPRESS_LEARN) {
				this->test_compression_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.01);
			} else {
				this->state_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.01);
			}
		} else {
			this->test_fold->backprop_last_state(errors, 0.002);

			vector<double> last_scope_state_errors(this->test_scope_sizes.back());
			for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
				this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
			}

			if (this->state == STATE_CAN_COMPRESS_LEARN
					|| this->state == STATE_COMPRESS_LEARN) {
				this->test_compression_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.002);
			} else {
				this->state_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.002);
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE
			|| this->state == STATE_ADD_SCOPE_TUNE) {
		if ((this->state_iter+1) % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		this->curr_fold->activate(flat_inputs,
								  state_vals);

		vector<double> errors;
		errors.push_back(target_val - this->curr_fold->output->acti_vals[0]);
		this->sum_error += abs(errors[0]);

		this->curr_fold->backprop_full_state(errors, 0.002);

		vector<vector<double>> state_errors(this->curr_scope_sizes.size());
		// state_errors[0][0] doesn't matter
		for (int sc_index = 1; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
			state_errors[sc_index].reserve(this->curr_scope_sizes[sc_index]);
			for (int st_index = 0; st_index < this->curr_scope_sizes[sc_index]; st_index++) {
				state_errors[sc_index].push_back(this->curr_fold->state_inputs[sc_index]->errors[st_index]);
				this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}

		for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
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

		this->state_network->backprop(state_errors.back(), 0.002);
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

		this->score_network->backprop(
			target_val,
			0.002);

		// state_errors[0][0] doesn't matter
		for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] += this->score_network->state_inputs[sc_index]->errors[st_index];
				this->score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}

		for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
			nodes[n_index]->backprop(target_val,
									 state_errors);
		}
	}

	increment();
}

void TestNode::increment() {
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

						this->test_scope_sizes = vector<int>{1};

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->fold_index++;
						this->test_fold->set_just_score();

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
			if (this->sum_error/10000 < 1.2*this->curr_fold->average_error) {
				cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_JUST_SCORE_TUNE" << endl;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->sum_error/10000 < this->curr_fold->average_error) {
					this->curr_fold->average_error = this->sum_error/10000;
				}

				this->compressed_scope_sizes.push_back(vector<int>());
				while (this->curr_scope_sizes.size() > 1) {
					this->compressed_scope_sizes[0].push_back(this->curr_scope_sizes.back());
					this->curr_scope_sizes.pop_back();
				}

				this->just_score = true;
				this->update_existing_scope = false;

				this->state = STATE_JUST_SCORE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				delete this->test_fold;
				this->test_fold = NULL;

				this->just_score = false;

				if (this->curr_scope_sizes.size() >= 2) {
					cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_LOCAL_SCOPE_LEARN" << endl;

					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->fold_index++;
					this->test_fold->reset_last();

					this->test_scope_sizes = this->curr_scope_sizes;

					this->state_network = new StateNetwork(this->test_scope_sizes,
														   this->obs_size,
														   this->test_scope_sizes.back());

					this->state = STATE_LOCAL_SCOPE_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_JUST_SCORE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_ADD_SCOPE_LEARN" << endl;

					this->update_existing_scope = false;

					this->new_scope_size = 1;

					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->fold_index++;
					this->test_fold->add_scope(this->new_scope_size);

					this->test_scope_sizes = this->curr_scope_sizes;
					this->test_scope_sizes.push_back(this->new_scope_size);

					this->state_network = new StateNetwork(this->curr_scope_sizes,
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
			if (this->sum_error/10000 < 1.2*this->curr_fold->average_error
					|| this->curr_fold->fold_index == (int)this->curr_fold->flat_sizes.size()-1) {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_LOCAL_SCOPE_TUNE" << endl;

				this->update_existing_scope = true;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->sum_error/10000 < this->curr_fold->average_error) {
					this->curr_fold->average_error = this->sum_error/10000;
				}

				this->curr_scope_sizes = this->test_scope_sizes;

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

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->fold_index++;
				this->test_fold->add_scope(this->new_scope_size);

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.push_back(this->new_scope_size);

				delete this->state_network;
				this->state_network = new StateNetwork(this->curr_scope_sizes,
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
					for (int sc_index = 1; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[sc_index];
					}
					if (sum_scope_sizes >= 2) {
						cout << "ending STATE_LOCAL_SCOPE_TUNE" << endl;
						cout << "starting STATE_CAN_COMPRESS_LEARN" << endl;

						this->test_scope_sizes = this->curr_scope_sizes;
						this->test_compress_num_scopes = (int)this->test_scope_sizes.size()-1;
						this->test_compressed_scope_sizes = vector<int>(this->test_scope_sizes.size()-1);
						for (int sc_index = (int)this->test_scope_sizes.size()-2; sc_index >= 0; sc_index--) {
							this->test_compressed_scope_sizes[sc_index] = this->test_scope_sizes.back();
							this->test_scope_sizes.pop_back();
						}
						this->test_compress_sizes = 1;
						this->test_scope_sizes.push_back(sum_scope_sizes-1);

						// use curr_scope_sizes for construction
						this->test_compression_network = new CompressionNetwork(this->curr_scope_sizes,
																				sum_scope_sizes-1);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->set_can_compress();

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

			delete this->test_fold;
			this->test_fold = NULL;

			if (this->sum_error/10000 < 1.2*this->curr_fold->average_error) {
				cout << "ending STATE_COMPRESS_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_COMPRESS_LEARN" << endl;

				this->test_scope_sizes = this->curr_scope_sizes;
				if (this->test_scope_sizes.back() == 1) {
					this->test_compress_num_scopes = 2;
					this->test_compress_sizes = 1;
					this->test_compressed_scope_sizes = vector<int>(2);
					int sum_scope_sizes = 0;
					this->test_compressed_scope_sizes[1] = this->test_scope_sizes.back();
					sum_scope_sizes += this->test_scope_sizes.back();
					this->test_scope_sizes.pop_back();
					this->test_compressed_scope_sizes[0] = this->test_scope_sizes.back();
					sum_scope_sizes += this->test_scope_sizes.back();
					this->test_scope_sizes.pop_back();
					this->test_scope_sizes.push_back(sum_scope_sizes-1);

					// use curr_scope_sizes for construction
					this->test_compression_network = new CompressionNetwork(this->curr_scope_sizes,
																			sum_scope_sizes-1);

					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->pop_scope();
					this->test_fold->pop_scope();
					this->test_fold->add_scope(sum_scope_sizes-1);
				} else {
					this->test_compress_num_scopes = 1;
					this->test_compress_sizes = 1;
					this->test_compressed_scope_sizes = vector<int>{this->test_scope_sizes.back()};
					int sum_scope_sizes = this->test_scope_sizes.back();
					this->test_scope_sizes.pop_back();
					this->test_scope_sizes.push_back(sum_scope_sizes-1);

					// use curr_scope_sizes for construction
					this->test_compression_network = new CompressionNetwork(this->curr_scope_sizes,
																			sum_scope_sizes-1);

					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->pop_scope();
					this->test_fold->add_scope(sum_scope_sizes-1);
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
			if (this->sum_error/10000 < this->curr_fold->average_error
					|| this->test_scope_sizes.size() == 2) {
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

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->sum_error/10000 < this->curr_fold->average_error) {
					this->curr_fold->average_error = this->sum_error/10000;
				}

				this->curr_scope_sizes = this->test_scope_sizes;

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

					this->test_scope_sizes = this->curr_scope_sizes;
					this->test_compress_num_scopes = this->compress_num_scopes.back();
					this->test_compress_sizes = this->compress_sizes.back()+1;
					this->test_compressed_scope_sizes = this->compressed_scope_sizes.back();
					this->test_scope_sizes.pop_back();
					this->test_scope_sizes.push_back(sum_scope_sizes-this->test_compress_sizes);

					this->test_compression_network = new CompressionNetwork(
						this->compression_networks.back()->scope_sizes,
						sum_scope_sizes-this->test_compress_sizes);

					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->pop_scope();
					this->test_fold->add_scope(sum_scope_sizes-this->test_compress_sizes);

					this->state = STATE_COMPRESS_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				delete this->test_compression_network;

				delete this->test_fold;
				this->test_fold = NULL;

				if (this->test_compress_sizes == 1) {
					cout << "ending STATE_COMPRESS_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_LEARN" << endl;

					this->test_compress_num_scopes++;
					this->test_scope_sizes.pop_back();
					this->test_compressed_scope_sizes.push_back(this->test_scope_sizes.back());

					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->test_compress_num_scopes; sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[this->curr_scope_sizes.size()-1-sc_index];
					}

					this->test_scope_sizes.pop_back();
					this->test_scope_sizes.push_back(sum_scope_sizes-1);

					this->test_compression_network = new CompressionNetwork(this->curr_scope_sizes,
																			sum_scope_sizes-1);

					this->test_fold = new FoldNetwork(this->curr_fold);
					for (int sc_index = 0; sc_index < this->test_compress_num_scopes; sc_index++) {
						this->test_fold->pop_scope();
					}
					this->test_fold->add_scope(sum_scope_sizes-1);

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
					for (int sc_index = 1; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[sc_index];
					}
					if (sum_scope_sizes >= 2) {
						cout << "ending STATE_COMPRESS_TUNE" << endl;
						cout << "starting STATE_CAN_COMPRESS_LEARN" << endl;

						this->test_scope_sizes = this->curr_scope_sizes;
						this->test_compress_num_scopes = (int)this->test_scope_sizes.size()-1;
						this->test_compressed_scope_sizes = vector<int>(this->test_scope_sizes.size()-1);
						for (int sc_index = (int)this->test_scope_sizes.size()-2; sc_index >= 0; sc_index--) {
							this->test_compressed_scope_sizes[sc_index] = this->test_scope_sizes.back();
							this->test_scope_sizes.pop_back();
						}
						this->test_compress_sizes = 1;
						this->test_scope_sizes.push_back(sum_scope_sizes-1);

						// use curr_scope_sizes for construction
						this->test_compression_network = new CompressionNetwork(this->curr_scope_sizes,
																				sum_scope_sizes-1);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->set_can_compress();

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
			if (this->sum_error/10000 < 1.2*this->curr_fold->average_error) {
				cout << "ending STATE_ADD_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_ADD_SCOPE_TUNE" << endl;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->sum_error/10000 < this->curr_fold->average_error) {
					this->curr_fold->average_error = this->sum_error/10000;
				}

				this->curr_scope_sizes = this->test_scope_sizes;

				this->state = STATE_ADD_SCOPE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_ADD_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_ADD_SCOPE_LEARN" << endl;

				this->new_scope_size++;

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->fold_index++;
				this->test_fold->add_scope(this->new_scope_size);

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.push_back(this->new_scope_size);

				delete this->state_network;
				this->state_network = new StateNetwork(this->curr_scope_sizes,
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
