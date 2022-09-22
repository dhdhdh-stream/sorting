#include "test_node.h"

#include <iostream>

using namespace std;

TestNode::TestNode(double target_error,
				   vector<int> initial_scope_sizes,
				   FoldNetwork* original_fold) {
	this->target_error = target_error;
	
	this->curr_scope_sizes = initial_scope_sizes;
	this->curr_fold = new FoldNetwork(original_fold);

	this->test_fold = new FoldNetwork(original_fold);
	this->test_fold->fold_index++;

	this->state_network = NULL;

	this->state = STATE_NO_OUTPUT_MEASURE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

TestNode::~TestNode() {
	// do nothing

	// all networks will be taken or cleared by increment()
}

void TestNode::activate(vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						double observation) {
	if (this->state == STATE_NO_OUTPUT_MEASURE) {
		// do nothing
	} else {
		vector<double> obs{observation};

		this->state_network->activate(state_vals,
									  scopes_on,
									  obs);
		if (state_vals.size() > 0) {
			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}

			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				this->compression_networks[c_index]->activate(state_vals,
															  scopes_on);

				state_vals.pop_back();
				scopes_on.pop_back();
				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
				}
			}
		}
	}
}

void TestNode::process(double* flat_inputs,
					   bool* activated,
					   vector<vector<double>>& state_vals,
					   double observation,
					   double target_val,
					   bool is_zero_train,
					   vector<Node*>& nodes) {
	vector<double> obs;
	obs.push_back(observation);

	if (this->state == STATE_NO_OUTPUT_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE) {
		this->test_fold->activate(flat_inputs,
								  activated,
								  obs,
								  state_vals);

		this->sum_error += abs(target_val - this->test_fold->output->acti_vals[0]);
	} else if (this->state == STATE_LOCAL_SCOPE_LEARN
			|| this->state == STATE_COMPRESS_LEARN) {
		if ((this->state_iter+1)%10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		if (this->state_iter <= 200000) {
			this->test_fold->activate(flat_inputs,
									  activated,
									  obs,
									  state_vals);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			// if (!is_zero_train) {
				this->sum_error += abs(errors[0]);
			// }

			this->test_fold->backprop_last_state(errors);

			if (this->test_scope_sizes.size() > 0) {
				vector<double> last_scope_state_errors(this->test_scope_sizes.back());
				for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
					this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->compression_networks.size() > 0) {
					this->compression_networks.back()->backprop_weights_with_no_error_signal(last_scope_state_errors);

					if (this->state_iter%100 == 0) {
						double max_update = 0.0;
						this->test_fold->calc_max_update_last_state(max_update, 0.001);
						this->compression_networks.back()->calc_max_update(max_update, 0.001);
						double factor = 1.0;
						if (max_update > 0.01) {
							factor = 0.01/max_update;
						}
						this->test_fold->update_weights_last_state(factor, 0.001);
						this->compression_networks.back()->update_weights(factor, 0.001);
					}
				} else {
					this->state_network->backprop_weights_with_no_error_signal(last_scope_state_errors);

					if (this->state_iter%100 == 0) {
						double max_update = 0.0;
						this->test_fold->calc_max_update_last_state(max_update, 0.001);
						this->state_network->calc_max_update(max_update, 0.001);
						double factor = 1.0;
						if (max_update > 0.01) {
							factor = 0.01/max_update;
						}
						this->test_fold->update_weights_last_state(factor, 0.001);
						this->state_network->update_weights(factor, 0.001);
					}
				}
			}
		} else {
			this->test_fold->activate(flat_inputs,
									  activated,
									  obs,
									  state_vals);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			// if (!is_zero_train) {
				this->sum_error += abs(errors[0]);
			// }

			this->test_fold->backprop_last_state(errors);

			if (this->test_scope_sizes.size() > 0) {
				vector<double> last_scope_state_errors(this->test_scope_sizes.back());
				for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
					this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->compression_networks.size() > 0) {
					this->compression_networks.back()->backprop_weights_with_no_error_signal(last_scope_state_errors);

					if (this->state_iter%100 == 0) {
						double max_update = 0.0;
						this->test_fold->calc_max_update_last_state(max_update, 0.0001);
						this->compression_networks.back()->calc_max_update(max_update, 0.0001);
						double factor = 1.0;
						if (max_update > 0.001) {
							factor = 0.001/max_update;
						}
						this->test_fold->update_weights_last_state(factor, 0.0001);
						this->compression_networks.back()->update_weights(factor, 0.0001);
					}
				} else {
					this->state_network->backprop_weights_with_no_error_signal(last_scope_state_errors);

					if (this->state_iter%100 == 0) {
						double max_update = 0.0;
						this->test_fold->calc_max_update_last_state(max_update, 0.0001);
						this->state_network->calc_max_update(max_update, 0.0001);
						double factor = 1.0;
						if (max_update > 0.001) {
							factor = 0.001/max_update;
						}
						this->test_fold->update_weights_last_state(factor, 0.0001);
						this->state_network->update_weights(factor, 0.0001);
					}
				}
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE) {
		if ((this->state_iter+1) % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		}

		this->test_fold->activate(flat_inputs,
								  activated,
								  obs,
								  state_vals);

		vector<double> errors;
		errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
		this->sum_error += abs(errors[0]);

		this->test_fold->backprop_full_state(errors);

		vector<vector<double>> state_errors(this->test_scope_sizes.size());
		for (int sc_index = 0; sc_index < (int)this->test_scope_sizes.size(); sc_index++) {
			state_errors[sc_index].reserve(this->test_scope_sizes[sc_index]);
			for (int st_index = 0; st_index < this->test_scope_sizes[sc_index]; st_index++) {
				state_errors[sc_index].push_back(this->test_fold->state_inputs[sc_index]->errors[st_index]);
				this->test_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}

		for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
			this->compression_networks[c_index]->backprop(state_errors.back());

			state_errors.push_back(vector<double>(this->compressed_scope_sizes[c_index]));
			
			for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index];
					this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
		}

		this->state_network->backprop(state_errors.back());
		for (int sc_index = 0; sc_index < (int)state_errors.size()-1; sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] += this->state_network->state_inputs[sc_index]->errors[st_index];
				this->state_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
		}
		for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
			state_errors.back()[st_index] = this->state_network->state_inputs.back()->errors[st_index];
			this->state_network->state_inputs.back()->errors[st_index] = 0.0;
		}

		for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
			nodes[n_index]->backprop(state_errors);
		}

		if (this->state_iter%100 == 0) {
			double max_update = 0.0;
			this->test_fold->calc_max_update_full_state(max_update, 0.0001);
			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				this->compression_networks[c_index]->calc_max_update(max_update, 0.0001);
			}
			this->state_network->calc_max_update(max_update, 0.0001);
			for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
				nodes[n_index]->calc_max_update(max_update, 0.0001);
			}
			double factor = 1.0;
			if (max_update > 0.001) {
				factor = 0.001/max_update;
			}
			this->test_fold->update_weights_full_state(factor, 0.0001);
			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				this->compression_networks[c_index]->update_weights(factor, 0.0001);
			}
			this->state_network->update_weights(factor, 0.0001);
			for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
				nodes[n_index]->update_weights(factor, 0.0001);
			}
		}
	}

	increment();
}

void TestNode::increment() {
	this->state_iter++;

	if (this->state == STATE_NO_OUTPUT_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < this->target_error) {
				cout << "ending STATE_NO_OUTPUT_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "DONE" << endl;

				this->outputs_state = false;
				this->update_existing_scope = false;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				this->state = STATE_DONE;
			} else {
				if (this->curr_scope_sizes.size() > 0) {
					cout << "ending STATE_NO_OUTPUT_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_LOCAL_SCOPE_LEARN" << endl;

					this->outputs_state = true;

					this->test_scope_sizes = this->curr_scope_sizes;

					this->state_network = new StateNetwork(this->test_scope_sizes);

					delete this->test_fold;
					this->test_fold = new FoldNetwork(this->curr_fold);
					this->test_fold->fold_index++;
					this->test_fold->reset_last();

					this->state = STATE_LOCAL_SCOPE_LEARN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_NO_OUTPUT_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "DONE" << endl;

					this->outputs_state = true;
					this->update_existing_scope = false;

					this->curr_scope_sizes.push_back(1);

					delete this->test_fold;
					this->curr_fold->add_scope(1);
					this->curr_fold->fold_index++;
					for (int n_index = 0; n_index < (int)this->curr_fold->hidden->acti_vals.size(); n_index++) {
						this->curr_fold->hidden->weights[n_index].back()[0] =
							this->curr_fold->hidden->weights[n_index][0][this->curr_fold->fold_index];
					}

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
			if (this->sum_error/10000 < this->target_error
					|| this->curr_fold->fold_index == this->curr_fold->flat_size-1) {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_LOCAL_SCOPE_TUNE" << endl;

				this->update_existing_scope = true;

				this->state = STATE_LOCAL_SCOPE_TUNE;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "DONE" << endl;

				this->update_existing_scope = false;

				this->curr_scope_sizes.push_back(1);

				delete this->state_network;
				this->state_network = NULL;

				delete this->test_fold;
				this->curr_fold->add_scope(1);
				this->curr_fold->fold_index++;
				for (int n_index = 0; n_index < (int)this->curr_fold->hidden->acti_vals.size(); n_index++) {
					this->curr_fold->hidden->weights[n_index].back()[0] =
						this->curr_fold->hidden->weights[n_index][0][this->curr_fold->fold_index];
				}

				this->state = STATE_DONE;
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE) {
		if (this->state_iter >= 50000) {
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
					delete this->curr_fold;
					this->curr_fold = this->test_fold;
					this->test_fold = NULL;

					this->curr_scope_sizes = this->test_scope_sizes;

					if (this->curr_scope_sizes.size() > 1) {
						cout << "ending STATE_LOCAL_SCOPE_TUNE" << endl;
						cout << "starting STATE_COMPRESS_LEARN" << endl;

						this->test_scope_sizes = this->curr_scope_sizes;
						this->test_scope_sizes.pop_back();

						// use curr_scope_sizes for construction
						this->compression_networks.push_back(new CompressionNetwork(this->curr_scope_sizes));
						this->compressed_scope_sizes.push_back(this->curr_scope_sizes.back());

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->reset_last();

						this->state = STATE_COMPRESS_LEARN;
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
			if (this->sum_error/10000 < this->target_error) {
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
				cout << "DONE" << endl;

				delete this->compression_networks.back();
				this->compression_networks.pop_back();

				delete this->test_fold;
				this->test_fold = NULL;

				this->state = STATE_DONE;
			}
		}
	} else if (this->state == STATE_COMPRESS_TUNE) {
		if (this->state_iter >= 50000) {
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
					delete this->curr_fold;
					this->curr_fold = this->test_fold;
					this->test_fold = NULL;

					this->curr_scope_sizes = this->test_scope_sizes;

					if (this->curr_scope_sizes.size() > 1) {
						cout << "ending STATE_COMPRESS_TUNE" << endl;
						cout << "starting STATE_COMPRESS_LEARN" << endl;

						this->test_scope_sizes = this->curr_scope_sizes;
						this->test_scope_sizes.pop_back();

						// use curr_scope_sizes for construction
						this->compression_networks.push_back(new CompressionNetwork(this->curr_scope_sizes));
						this->compressed_scope_sizes.push_back(this->curr_scope_sizes.back());

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->reset_last();

						this->state = STATE_COMPRESS_LEARN;
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
	}
}
