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
						double observation,
						vector<vector<double>>& test_state_vals) {
	test_state_vals = state_vals;

	if (this->state == STATE_NO_OUTPUT_MEASURE) {
		// do nothing
	} else if (this->state == STATE_NEW_SCOPE_LEARN_FOLD) {
		test_state_vals.push_back(vector<double>{observation});
	} else {
		vector<double> inputs;
		inputs.push_back(observation);
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
				inputs.push_back(state_vals[sc_index][st_index]);
			}
		}
		this->state_network->activate(inputs);
		if (test_state_vals.size() > 0) {
			for (int st_index = 0; st_index < (int)test_state_vals.back().size(); st_index++) {
				test_state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}

			for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
				vector<double> inputs;
				for (int sc_index = 0; sc_index < (int)test_state_vals.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)test_state_vals[sc_index].size(); st_index++) {
						inputs.push_back(test_state_vals[sc_index][st_index]);
					}
				}
				test_state_vals.pop_back();

				this->compression_networks[c_index]->activate(inputs);

				for (int st_index = 0; st_index < (int)test_state_vals.back().size(); st_index++) {
					test_state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
				}
			}
		}
	}
}

void TestNode::process(double* flat_inputs,
					   bool* activated,
					   vector<vector<double>>& state_vals,
					   vector<vector<double>>& test_state_vals,
					   double observation,
					   double target_val) {
	vector<double> obs;
	obs.push_back(rand()%2);

	if (this->state == STATE_NO_OUTPUT_MEASURE
			|| this->state == STATE_LOCAL_SCOPE_MEASURE
			|| this->state == STATE_COMPRESS_MEASURE) {
		this->test_fold->activate_full(flat_inputs,
									   activated,
									   obs,
									   test_state_vals);

		this->sum_error += abs(target_val - this->test_fold->output->acti_vals[0]);
	} else if (this->state == STATE_LOCAL_SCOPE_LEARN
			|| this->state == STATE_COMPRESS_LEARN) {
		if (this->state_iter % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		if (this->state_iter < 100000) {
			this->curr_fold->activate_hidden_linear(flat_inputs,
													activated,
													obs,
													state_vals);
			this->test_fold->activate_hidden_linear(flat_inputs,
													activated,
													obs,
													test_state_vals);
			for (int n_index = 0; n_index < (int)this->test_fold->hidden->acti_vals.size(); n_index++) {
				this->test_fold->hidden->errors[n_index] = this->curr_fold->hidden->acti_vals[n_index]
					- this->test_fold->hidden->acti_vals[n_index];
				this->sum_error += abs(this->test_fold->hidden->errors[n_index]);
			}
			this->test_fold->backprop_hidden_linear();

			if (this->test_scope_sizes.size() > 0) {
				vector<double> last_scope_state_errors(this->test_scope_sizes.back());
				for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
					this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->compression_networks.size() > 0) {
					this->compression_networks.back()->backprop(last_scope_state_errors);
				} else {
					this->state_network->backprop(last_scope_state_errors);
				}
			}
		} else {
			this->test_fold->activate_full(flat_inputs,
										   activated,
										   obs,
										   test_state_vals);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			this->sum_error += abs(errors[0]);

			this->test_fold->backprop_full(errors);

			if (this->test_scope_sizes.size() > 0) {
				vector<double> last_scope_state_errors(this->test_scope_sizes.back());
				for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
					this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->compression_networks.size() > 0) {
					this->compression_networks.back()->backprop(last_scope_state_errors);
				} else {
					this->state_network->backprop(last_scope_state_errors);
				}
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_ZERO_TRAIN
			|| this->state == STATE_COMPRESS_ZERO_TRAIN) {
		if (this->state_iter % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		this->test_fold->activate_full(flat_inputs,
									   activated,
									   obs,
									   test_state_vals);

		vector<double> errors;
		errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
		this->sum_error += abs(errors[0]);

		this->test_fold->backprop_full(errors);

		vector<double> last_scope_state_errors(this->test_scope_sizes.back());
		for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
			last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
			this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
		}

		if (this->compression_networks.size() > 0) {
			this->compression_networks.back()->backprop(last_scope_state_errors);
		} else {
			this->state_network->backprop(last_scope_state_errors);
		}
	} else if (this->state == STATE_NEW_SCOPE_LEARN_FOLD) {
		if (this->state_iter % 10000 == 0) {
			cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		if (this->state_iter < 50000) {
			this->curr_fold->activate_hidden_linear(flat_inputs,
													activated,
													obs,
													state_vals);
			this->test_fold->activate_hidden_linear(flat_inputs,
													activated,
													obs,
													test_state_vals);
			for (int n_index = 0; n_index < (int)this->test_fold->hidden->acti_vals.size(); n_index++) {
				this->test_fold->hidden->errors[n_index] = this->curr_fold->hidden->acti_vals[n_index]
					- this->test_fold->hidden->acti_vals[n_index];
				this->sum_error += abs(this->test_fold->hidden->errors[n_index]);
			}
			this->test_fold->backprop_hidden_linear();
		} else {
			this->test_fold->activate_full(flat_inputs,
										   activated,
										   obs,
										   test_state_vals);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			this->sum_error += abs(errors[0]);

			this->test_fold->backprop(errors);
		}
	}

	increment();
}

void TestNode::process_zero_train(vector<vector<double>>& state_vals,
								  vector<vector<double>>& zero_train_state_vals,
								  double observation) {
	if (this->state_iter % 10000 == 0) {
		cout << this->state_iter << " sum_error: " << this->sum_error << endl;
		this->sum_error = 0.0;
	}

	vector<vector<double>> test_state_vals;
	activate(state_vals, observation, test_state_vals);

	vector<vector<double>> zero_train_test_state_vals;
	activate(zero_train_state_vals, observation, zero_train_test_state_vals);

	vector<double> last_scope_state_errors(this->curr_scope_sizes.back());
	for (int st_index = 0; st_index < this->curr_scope_sizes.back(); st_index++) {
		last_scope_state_errors[st_index] = test_state_vals.back()[st_index]
			- zero_train_test_state_vals.back()[st_index];
		this->sum_error += abs(last_scope_state_errors[st_index]);
	}

	if (this->compression_networks.size() > 0) {
		this->compression_networks.back()->backprop(last_scope_state_errors);
	} else {
		this->state_network->backprop(last_scope_state_errors);
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

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				this->state = STATE_DONE;
			} else {
				cout << "ending STATE_NO_OUTPUT_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_LOCAL_SCOPE_LEARN" << endl;

				this->outputs_state = true;

				this->test_scope_sizes = this->curr_scope_sizes;
				int total_state_size = 0;
				for (int sc_index = 0; sc_index < (int)this->test_scope_sizes.size(); sc_index++) {
					total_state_size += this->test_scope_sizes[sc_index];
				}
				
				this->state_network = new Network(1+total_state_size,
												  8*(1+total_state_size),
												  total_state_size);

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->fold_index++;

				this->state = STATE_LOCAL_SCOPE_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_LOCAL_SCOPE_LEARN) {
		if (this->state_iter >= 500000) {
			cout << "ending STATE_LOCAL_SCOPE_LEARN" << endl;
			cout << "starting STATE_LOCAL_SCOPE_MEASURE" << endl;

			this->state = STATE_LOCAL_SCOPE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_LOCAL_SCOPE_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < this->target_error) {
				this->update_existing_scope = true;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				this->curr_scope_sizes = this->test_scope_sizes;

				if (this->curr_scope_sizes.size() > 1) {
					cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_LOCAL_SCOPE_ZERO_TRAIN" << endl;

					this->state = STATE_LOCAL_SCOPE_ZERO_TRAIN;
					this->state_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "DONE" << endl;

					this->state = STATE_DONE;
				}
			} else {
				cout << "ending STATE_LOCAL_SCOPE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_NEW_SCOPE_LEARN_FOLD" << endl;

				this->update_existing_scope = false;

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.push_back(1);
				int total_state_size = 0;
				for (int sc_index = 0; sc_index < (int)this->test_scope_sizes.size(); sc_index++) {
					total_state_size += this->test_scope_sizes[sc_index];
				}

				delete this->state_network;
				this->state_network = NULL;

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->add_scope(1);

				this->state = STATE_NEW_SCOPE_LEARN_FOLD;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (STATE_LOCAL_SCOPE_ZERO_TRAIN) {
		if (this->state_iter >= 500000*(int)this->curr_scope_sizes.size()) {
			if (this->curr_scope_sizes.size() > 1) {
				cout << "ending STATE_LOCAL_SCOPE_ZERO_TRAIN" << endl;
				cout << "starting STATE_COMPRESS_LEARN" << endl;

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.pop_back();

				int starting_state_size = 0;
				for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
					starting_state_size += this->curr_scope_sizes[sc_index];
				}
				this->compression_networks.push_back(new Network(starting_state_size,
																 4*(starting_state_size+this->test_scope_sizes.back()),
																 this->test_scope_sizes.back()));

				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->pop_scope();

				this->state = STATE_COMPRESS_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_LOCAL_SCOPE_ZERO_TRAIN" << endl;
				cout << "DONE" << endl;

				this->state = STATE_DONE;
			}
		}
	} else if (this->state == STATE_COMPRESS_LEARN) {
		if (this->state_iter >= 500000) {
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
				cout << "starting STATE_COMPRESS_ZERO_TRAIN" << endl;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				this->curr_scope_sizes = this->test_scope_sizes;

				this->state = STATE_COMPRESS_ZERO_TRAIN;
				this->state_iter = 0;
				this->sum_error = 0.0;
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
	} else if (this->state == STATE_COMPRESS_ZERO_TRAIN) {
		if (this->state_iter >= 500000*(int)this->curr_scope_sizes.size()) {
			if (this->curr_scope_sizes.size() > 1) {
				cout << "ending STATE_COMPRESS_ZERO_TRAIN" << endl;
				cout << "starting STATE_COMPRESS_LEARN" << endl;

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.pop_back();

				int starting_state_size = 0;
				for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
					starting_state_size += this->curr_scope_sizes[sc_index];
				}
				this->compression_networks.push_back(new Network(starting_state_size,
																 4*(starting_state_size+this->test_scope_sizes.back()),
																 this->test_scope_sizes.back()));

				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->pop_scope();

				this->state = STATE_COMPRESS_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_COMPRESS_ZERO_TRAIN" << endl;
				cout << "DONE" << endl;

				this->state = STATE_DONE;
			}
		}
	} else if (this->state == STATE_NEW_SCOPE_LEARN_FOLD) {
		if (this->state_iter >= 100000) {
			cout << "ending STATE_NEW_SCOPE_LEARN_FOLD" << endl;
			cout << "DONE" << endl;

			delete this->curr_fold;
			this->curr_fold = this->test_fold;
			this->test_fold = NULL;

			this->state = STATE_DONE;
		}
	}
}
