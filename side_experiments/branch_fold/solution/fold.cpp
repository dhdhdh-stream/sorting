#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(string id,
		   int sequence_length,
		   vector<Scope*> compound_actions,
		   vector<int> obs_sizes,
		   double average_score,
		   double original_flat_error,
		   FoldNetwork* original_fold,
		   vector<FoldNetwork*> original_input_folds) {
	this->id = id;

	this->sequence_length = sequence_length;
	this->compound_actions = compound_actions;
	this->obs_sizes = obs_sizes;
	this->average_score = average_score;
	this->original_flat_error = original_flat_error;

	this->curr_fold = original_fold;
	this->curr_scope_input_folds = original_input_folds;

	this->test_fold = NULL;
	this->test_scope_input_folds = vector<FoldNetwork*>(original_input_folds.size(), NULL);

	this->curr_action_input_network = NULL;
	this->test_action_input_network = NULL;
	this->small_action_input_network = NULL;

	this->curr_score_network = NULL;
	this->test_score_network = NULL;
	this->small_score_network = NULL;

	this->curr_compression_network = NULL;
	this->compress_size = 0;
	this->compress_num_layers = 0;
	this->compress_new_size = 0;
	this->test_compression_network = NULL;
	this->small_compression_network = NULL;

	if (this->compound_actions[0] == NULL) {
		cout << "starting STATE_OBS" << endl;

		this->action = NULL;
		this->small_action_input_network = NULL;

		this->new_layer_size = 0;
		this->obs_network = NULL;

		this->test_fold = new FoldNetwork(this->curr_fold);
		this->test_fold->fold_index++;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->compound_actions[f_index] != NULL
					&& this->compound_actions[f_index]->num_inputs > 0) {
				this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
				this->test_scope_input_folds[f_index]->fold_index++;
			}
		}

		this->state = STATE_OBS;
		this->stage = STAGE_LEARN;
		this->stage_iter = 0;
		this->sum_error = 0.0;
		this->new_state_factor = 25;
	} else if (this->compound_actions[0] != NULL
			&& this->compound_actions[0]->num_inputs > 0) {
		cout << "starting STATE_INNER_SCOPE_INPUT_SMALL" << endl;

		this->action = this->compound_actions[0];
		// this->curr_scope_sizes.size() == 0
		// edge case
		this->small_action_input_network = new SmallNetwork(0, 0, 0, this->compound_actions[0]->num_inputs);

		this->state = STATE_INNER_SCOPE_INPUT_SMALL;
		this->stage_iter = 0;
		this->sum_error = 0.0;
	} else {
		// this->compound_actions[0] != NULL && this->compound_actions[0]->num_inputs == 0
		cout << "starting STATE_INNER_SCOPE" << endl;

		this->action = this->compound_actions[0];
		this->small_action_input_network = NULL;

		this->state = STATE_INNER_SCOPE;
		this->stage_iter = 0;
		this->sum_error = 0.0;
	}
}

Fold::~Fold() {
	// do nothing
}

void Fold::process(vector<vector<double>>& flat_vals,
				   vector<vector<vector<double>>>& inner_flat_vals,
				   double target_val) {
	switch(this->state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step(flat_vals,
								   inner_flat_vals,
								   target_val);
			break;
		case STATE_INNER_SCOPE_INPUT_INPUT:
			inner_scope_input_input_step(flat_vals,
										 inner_flat_vals,
										 target_val);
			break;
		case STATE_INNER_SCOPE_INPUT_SMALL:
			inner_scope_input_small_step(flat_vals,
										 inner_flat_vals,
										 target_val);
			break;
		case STATE_INNER_SCOPE:
			inner_scope_step(flat_vals,
							 inner_flat_vals,
							 target_val);
			break;
		case STATE_OBS:
			obs_step(flat_vals,
					 inner_flat_vals,
					 target_val);
			break;
		case STATE_SCORE:
			score_step(flat_vals,
					   inner_flat_vals,
					   target_val);
			break;
		case STATE_SCORE_INPUT:
			score_input_step(flat_vals,
							 inner_flat_vals,
							 target_val);
			break;
		case STATE_SCORE_SMALL:
			score_small_step(flat_vals,
							 inner_flat_vals,
							 target_val);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step(flat_vals,
							inner_flat_vals,
							target_val);
			break;
		case STATE_COMPRESS_STATE:
			compress_step(flat_vals,
						  inner_flat_vals,
						  target_val);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step(flat_vals,
						  inner_flat_vals,
						  target_val);
			break;
		case STATE_COMPRESS_INPUT:
			compress_input_step(flat_vals,
								inner_flat_vals,
								target_val);
			break;
		case STATE_COMPRESS_SMALL:
			compress_small_step(flat_vals,
								inner_flat_vals,
								target_val);
			break;
		case STATE_FINAL_TUNE:
			final_tune_step(flat_vals,
							inner_flat_vals,
							target_val);
			break;
	}

	increment();
}

void Fold::increment() {
	this->stage_iter++;

	if (this->state == STATE_INNER_SCOPE_INPUT) {
		if (this->stage_iter >= 200000) {
			cout << "ending STATE_INNER_SCOPE_INPUT" << endl;

			if (this->curr_scope_sizes.size() == 1) {
				cout << "starting STATE_INNER_SCOPE_INPUT_SMALL" << endl;

				delete this->curr_action_input_network;
				int hidden_size = 5*(this->curr_scope_sizes.back()+this->curr_s_input_sizes.back());
				hidden_size = max(min(hidden_size, 50), 10);
				this->small_action_input_network = new SmallNetwork(this->curr_scope_sizes.back(),
																	this->curr_s_input_sizes.back(),
																	hidden_size,
																	this->compound_actions[this->nodes.size()]->num_inputs);

				this->state = STATE_INNER_SCOPE_INPUT_SMALL;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "starting STATE_INNER_SCOPE_INPUT_INPUT" << endl;

				this->test_action_input_network = new SubFoldNetwork(this->curr_action_input_network);
				this->test_action_input_network->fold_index++;

				this->state = STATE_INNER_SCOPE_INPUT_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_INNER_SCOPE_INPUT_INPUT
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 200000) {
			this->state = STATE_INNER_SCOPE_INPUT_INPUT;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_INNER_SCOPE_INPUT_INPUT
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < 0.0001
					|| (this->score_input_networks.size() > 0
						&& this->score_input_sizes.back() == (this->curr_scope_sizes[this->score_input_layer.back()]
							+ this->curr_s_input_sizes[this->score_input_layer.back()]))) {
				delete this->curr_action_input_network;
				this->curr_action_input_network = this->test_action_input_network;
				this->test_action_input_network = NULL;

				if (this->curr_action_input_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					cout << "ending STATE_INNER_SCOPE_INPUT_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_INNER_SCOPE_INPUT_SMALL" << endl;

					delete this->curr_action_input_network;
					int hidden_size = 5*(this->curr_scope_sizes.back()+this->curr_s_input_sizes.back());
					hidden_size = max(min(hidden_size, 50), 10);
					this->small_action_input_network = new SmallNetwork(this->curr_scope_sizes.back(),
																		this->curr_s_input_sizes.back(),
																		hidden_size,
																		this->compound_actions[this->nodes.size()]->num_inputs);

					this->state = STATE_INNER_SCOPE_INPUT_SMALL;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_INNER_SCOPE_INPUT_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_INNER_SCOPE_INPUT_INPUT" << endl;

					this->test_action_input_network = new SubFoldNetwork(this->curr_action_input_network);
					this->test_action_input_network->fold_index++;

					// start from size 1 if have previous
					if (this->action_input_input_networks.size() > 0) {
						this->action_input_input_layer.push_back(this->test_action_input_network->fold_index);
						this->action_input_input_sizes.push_back(1);
						int hidden_size = 5*(this->curr_scope_sizes[this->test_action_input_network->fold_index]
							+ this->curr_s_input_sizes[this->test_action_input_network->fold_index]);
						hidden_size = max(min(hidden_size, 50), 10);
						this->action_input_input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_action_input_network->fold_index],
																					 this->curr_s_input_sizes[this->test_action_input_network->fold_index],
																					 hidden_size,
																					 1));
						this->test_action_input_network->add_s_input(this->test_action_input_network->fold_index+1,
																	 1);
						// add to curr_s_input_sizes permanently 1 at a time
						this->curr_s_input_sizes[this->test_action_input_network->fold_index+1]++;
					}

					this->state = STATE_INNER_SCOPE_INPUT_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				cout << "ending STATE_INNER_SCOPE_INPUT_INPUT" << endl;
				cout << "FAILURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_INNER_SCOPE_INPUT_INPUT" << endl;

				delete this->test_action_input_network;
				this->test_action_input_network = new SubFoldNetwork(this->curr_action_input_network);
				this->test_action_input_network->fold_index++;

				if (this->action_input_input_layer.size() > 0
						&& this->action_input_input_layer.back() == this->test_action_input_network->fold_index) {
					this->action_input_input_sizes.back()++;
					delete this->action_input_input_networks.back();
					this->action_input_input_networks.pop_back();
				} else {
					this->action_input_input_layer.push_back(this->test_action_input_network->fold_index);
					this->action_input_input_sizes.push_back(1);
				}
				int hidden_size = 5*(this->curr_scope_sizes[this->test_action_input_network->fold_index]
					+ this->curr_s_input_sizes[this->test_action_input_network->fold_index]);
				hidden_size = max(min(hidden_size, 50), 10);
				this->action_input_input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_action_input_network->fold_index],
																			 this->curr_s_input_sizes[this->test_action_input_network->fold_index],
																			 hidden_size,
																			 this->action_input_input_sizes.back()));
				this->test_action_input_network->add_s_input(this->test_action_input_network->fold_index+1,
															 this->action_input_input_sizes.back());
				// add to curr_s_input_sizes permanently 1 at a time
				this->curr_s_input_sizes[this->test_action_input_network->fold_index+1]++;

				this->state = STATE_INNER_SCOPE_INPUT_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_INNER_SCOPE_INPUT_SMALL) {
		if (this->stage_iter >= 200000) {
			cout << "ending STATE_INNER_SCOPE_INPUT_SMALL" << endl;
			cout << "starting STATE_INNER_SCOPE" << endl;

			this->state = STATE_INNER_SCOPE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_INNER_SCOPE) {
		if (this->stage_iter >= 400000) {
			cout << "ending STATE_INNER_SCOPE" << endl;
			cout << "starting STATE_OBS" << endl;

			this->new_layer_size = 0;
			this->obs_network = NULL;

			this->test_scope_sizes = this->curr_scope_sizes;
			this->test_s_input_sizes = this->curr_s_input_sizes;
			this->test_fold = new FoldNetwork(this->curr_fold);
			this->test_fold->fold_index++;
			for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
				if (this->compound_actions[f_index] != NULL
						&& this->compound_actions[f_index]->num_inputs > 0) {
					// test_scope_input_folds not initialized
					this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
					this->test_scope_input_folds[f_index]->fold_index++;
				}
			}

			this->state = STATE_OBS;
			this->stage = STAGE_LEARN;
			this->stage_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else if (this->state == STATE_OBS
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter == 30000) {
			this->new_state_factor = 5;
		} else if (this->stage_iter == 60000) {
			this->new_state_factor = 1;
		}

		if (this->stage_iter >= 200000) {
			this->state = STATE_OBS;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_OBS
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			bool is_max_size;
			if (this->compound_actions[this->nodes.size()] == NULL) {
				is_max_size = (this->new_layer_size == this->obs_sizes[this->nodes.size()]);
			} else {
				is_max_size = (this->new_layer_size == this->compound_actions[this->nodes.size()]->num_outputs);
			}
			if (this->sum_error/10000 < 1.02*this->original_flat_error || is_max_size) {
				this->curr_scope_sizes = this->test_scope_sizes;
				this->curr_s_input_sizes = this->test_s_input_sizes;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;
				for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->curr_scope_input_folds[f_index];
						this->curr_scope_input_folds[f_index] = this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = NULL;
					}
				}

				if (this->new_layer_size == 0) {
					cout << "ending STATE_OBS" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					
					int obs_size;
					if (this->compound_actions[this->nodes.size()] == NULL) {
						obs_size = this->obs_sizes[this->nodes.size()];
					} else {
						obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
					}
					Node* new_node = new Node(this->id+"_n_"+to_string(this->nodes.size()),
											  (this->compound_actions[this->nodes.size()] != NULL),
											  this->action_input_input_layer,
											  this->action_input_input_sizes,
											  this->action_input_input_networks,
											  this->small_action_input_network,
											  this->action,
											  obs_size,
											  this->new_layer_size,
											  this->obs_network,
											  this->score_input_layer,
											  this->score_input_sizes,
											  this->score_input_networks,
											  this->small_score_network,
											  this->compress_num_layers,
											  this->compress_new_size,
											  this->input_layer,
											  this->input_sizes,
											  this->input_networks,
											  this->small_compression_network,
											  this->compressed_scope_sizes,
											  this->compressed_s_input_sizes);
					this->nodes.push_back(new_node);

					if ((int)this->nodes.size() == this->sequence_length) {
						cout << "DONE" << endl;

						this->state = STATE_DONE;
					} else {
						this->curr_action_input_network = NULL;
						this->test_action_input_network = NULL;
						this->action_input_input_layer.clear();
						this->action_input_input_sizes.clear();
						this->action_input_input_networks.clear();
						this->small_action_input_network = NULL;

						if (this->compound_actions[this->nodes.size()] == NULL) {
							cout << "starting STATE_OBS" << endl;

							this->action = NULL;
							this->small_action_input_network = NULL;

							this->new_layer_size = 0;
							this->obs_network = NULL;

							this->test_fold = new FoldNetwork(this->curr_fold);
							this->test_fold->fold_index++;
							for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
								if (this->compound_actions[f_index] != NULL
										&& this->compound_actions[f_index]->num_inputs > 0) {
									this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
									this->test_scope_input_folds[f_index]->fold_index++;
								}
							}

							this->state = STATE_OBS;
							this->stage = STAGE_LEARN;
							this->stage_iter = 0;
							this->sum_error = 0.0;
							this->new_state_factor = 25;
						} else if (this->compound_actions[this->nodes.size()] != NULL
								&& this->compound_actions[this->nodes.size()]->num_inputs > 0) {
							this->action = this->compound_actions[this->nodes.size()];
							if (this->curr_scope_sizes.size() == 0) {
								cout << "starting STATE_INNER_SCOPE_INPUT_SMALL" << endl;

								// edge case
								this->small_action_input_network = new SmallNetwork(0, 0, 0, this->compound_actions[this->nodes.size()]->num_inputs);

								this->state = STATE_INNER_SCOPE_INPUT_SMALL;
								this->stage_iter = 0;
								this->sum_error = 0.0;
							} else {
								cout << "starting STATE_INNER_SCOPE_INPUT" << endl;

								this->curr_action_input_network = new SubFoldNetwork(this->curr_scope_sizes,
																					 this->curr_s_input_sizes,
																					 this->compound_actions[this->nodes.size()]->num_inputs);

								this->state = STATE_INNER_SCOPE_INPUT;
								this->stage_iter = 0;
								this->sum_error = 0.0;
							}
						} else {
							// this->compound_actions[this->nodes.size()] != NULL && this->compound_actions[this->nodes.size()]->num_inputs == 0
							cout << "starting STATE_INNER_SCOPE" << endl;

							this->action = this->compound_actions[this->nodes.size()];
							this->small_action_input_network = NULL;

							this->state = STATE_INNER_SCOPE;
							this->stage_iter = 0;
							this->sum_error = 0.0;
						}
					}
				} else {
					cout << "ending STATE_OBS" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_SCORE" << endl;

					this->test_score_network = new SubFoldNetwork(this->curr_scope_sizes,
																  this->curr_s_input_sizes,
																  1);

					this->state = STATE_SCORE;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				cout << "ending STATE_OBS" << endl;
				cout << "FAILURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_OBS" << endl;

				this->new_layer_size++;

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.push_back(this->new_layer_size);
				this->test_s_input_sizes = this->curr_s_input_sizes;
				this->test_s_input_sizes.push_back(0);

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->fold_index++;
				this->test_fold->add_scope(this->new_layer_size);
				for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
						this->test_scope_input_folds[f_index]->fold_index++;
						this->test_scope_input_folds[f_index]->add_scope(this->new_layer_size);
					}
				}

				if (this->obs_network != NULL) {
					delete this->obs_network;
				}
				int obs_size;
				if (this->compound_actions[this->nodes.size()] == NULL) {
					obs_size = this->obs_sizes[this->nodes.size()];
				} else {
					obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
				}
				int hidden_size = 5*obs_size;
				hidden_size = max(min(hidden_size, 50), 10);
				this->obs_network = new Network(obs_size,
												hidden_size,
												this->new_layer_size);

				this->state = STATE_OBS;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			}
		}
	} else if (this->state == STATE_SCORE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 200000) {
			this->state = STATE_SCORE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_SCORE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			this->curr_score_network = this->test_score_network;

			if (this->curr_scope_sizes.size() == 1) {
				cout << "ending STATE_SCORE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_SMALL" << endl;

				delete this->curr_score_network;
				int hidden_size = 5*(this->curr_scope_sizes.back()+this->curr_s_input_sizes.back());
				hidden_size = max(min(hidden_size, 50), 10);
				this->small_score_network = new SmallNetwork(this->curr_scope_sizes.back(),
															 this->curr_s_input_sizes.back(),
															 hidden_size,
															 1);

				this->state = STATE_SCORE_SMALL;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_SCORE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_INPUT" << endl;

				this->average_misguess = this->sum_error/10000;

				this->test_score_network = new SubFoldNetwork(this->curr_score_network);
				this->test_score_network->fold_index++;

				this->state = STATE_SCORE_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE_INPUT
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 200000) {
			this->state = STATE_SCORE_INPUT;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_SCORE_INPUT
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < 1.02*this->average_misguess
					|| (this->score_input_networks.size() > 0
						&& this->score_input_sizes.back() == (this->curr_scope_sizes[this->score_input_layer.back()]
							+ this->curr_s_input_sizes[this->score_input_layer.back()]))) {
				if (this->curr_score_network != NULL) {
					delete this->curr_score_network;
				}
				this->curr_score_network = this->test_score_network;
				this->test_score_network = NULL;

				if (this->curr_score_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					cout << "ending STATE_SCORE_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_SCORE_SMALL" << endl;

					delete this->curr_score_network;
					int hidden_size = 5*(this->curr_scope_sizes.back()+this->curr_s_input_sizes.back());
					hidden_size = max(min(hidden_size, 50), 10);
					this->small_score_network = new SmallNetwork(this->curr_scope_sizes.back(),
																 this->curr_s_input_sizes.back(),
																 hidden_size,
																 1);

					this->state = STATE_SCORE_SMALL;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_SCORE_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_SCORE_INPUT" << endl;

					this->test_score_network = new SubFoldNetwork(this->curr_score_network);
					this->test_score_network->fold_index++;

					// start from size 1 if have previous
					if (this->score_input_networks.size() > 0) {
						this->score_input_layer.push_back(this->test_score_network->fold_index);
						this->score_input_sizes.push_back(1);
						int hidden_size = 5*(this->curr_scope_sizes[this->test_score_network->fold_index]
							+ this->curr_s_input_sizes[this->test_score_network->fold_index]);
						hidden_size = max(min(hidden_size, 50), 10);
						this->score_input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_score_network->fold_index],
																			  this->curr_s_input_sizes[this->test_score_network->fold_index],
																			  hidden_size,
																			  1));
						this->test_score_network->add_s_input(this->test_score_network->fold_index+1,
															  1);
						// add to curr_s_input_sizes permanently 1 at a time
						this->curr_s_input_sizes[this->test_score_network->fold_index+1]++;
					}

					this->state = STATE_SCORE_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				cout << "ending STATE_SCORE_INPUT" << endl;
				cout << "FAILURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_INPUT" << endl;

				delete this->test_score_network;
				this->test_score_network = new SubFoldNetwork(this->curr_score_network);
				this->test_score_network->fold_index++;

				if (this->score_input_layer.size() > 0
						&& this->score_input_layer.back() == this->test_score_network->fold_index) {
					this->score_input_sizes.back()++;
					delete this->score_input_networks.back();
					this->score_input_networks.pop_back();
				} else {
					this->score_input_layer.push_back(this->test_score_network->fold_index);
					this->score_input_sizes.push_back(1);
				}
				int hidden_size = 5*(this->curr_scope_sizes[this->test_score_network->fold_index]
					+ this->curr_s_input_sizes[this->test_score_network->fold_index]);
				hidden_size = max(min(hidden_size, 50), 10);
				this->score_input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_score_network->fold_index],
																	  this->curr_s_input_sizes[this->test_score_network->fold_index],
																	  hidden_size,
																	  this->score_input_sizes.back()));
				this->test_score_network->add_s_input(this->test_score_network->fold_index+1,
													  this->score_input_sizes.back());
				// add to curr_s_input_sizes permanently 1 at a time
				this->curr_s_input_sizes[this->test_score_network->fold_index+1]++;

				this->state = STATE_SCORE_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE_SMALL) {
		if (this->stage_iter >= 200000) {
			cout << "ending STATE_SCORE_SMALL" << endl;
			cout << "starting STATE_SCORE_TUNE" << endl;

			this->state = STATE_SCORE_TUNE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_SCORE_TUNE) {
		if (this->stage_iter >= 200000) {
			cout << "ending STATE_SCORE_TUNE" << endl;
			cout << "starting STATE_COMPRESS_STATE" << endl;

			int sum_scope_sizes = 0;
			for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
				sum_scope_sizes += this->curr_scope_sizes[sc_index];
			}

			this->test_scope_sizes = this->curr_scope_sizes;
			this->test_s_input_sizes = this->curr_s_input_sizes;
			this->compress_num_layers = (int)this->curr_scope_sizes.size();
			this->compressed_scope_sizes = vector<int>(this->curr_scope_sizes.size());
			this->compressed_s_input_sizes = vector<int>(this->curr_s_input_sizes.size());
			for (int sc_index = (int)this->compressed_scope_sizes.size()-1; sc_index >= 0; sc_index--) {
				this->compressed_scope_sizes[sc_index] = this->test_scope_sizes.back();
				this->test_scope_sizes.pop_back();

				this->compressed_s_input_sizes[sc_index] = this->test_s_input_sizes.back();
				this->test_s_input_sizes.pop_back();
			}
			this->compress_size = 1;
			this->compress_new_size = sum_scope_sizes-1;

			if (this->compress_new_size == 0) {
				this->test_fold = new FoldNetwork(this->curr_fold);
				while (this->test_fold->state_inputs.size() > 0) {
					this->test_fold->pop_scope();
				}
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
						while (this->test_scope_input_folds[f_index]->state_inputs.size() > 0) {
							this->test_scope_input_folds[f_index]->pop_scope();
						}
					}
				}
			} else {
				this->test_scope_sizes.push_back(this->compress_new_size);
				this->test_s_input_sizes.push_back(0);

				// use curr_scope_sizes for construction
				this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																	this->curr_s_input_sizes,
																	this->compress_new_size);

				this->test_fold = new FoldNetwork(this->curr_fold);
				while (this->test_fold->state_inputs.size() > 0) {
					this->test_fold->pop_scope();
				}
				this->test_fold->add_scope(this->compress_new_size);
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
						while (this->test_scope_input_folds[f_index]->state_inputs.size() > 0) {
							this->test_scope_input_folds[f_index]->pop_scope();
						}
						this->test_scope_input_folds[f_index]->add_scope(this->compress_new_size);
					}
				}
			}

			this->state = STATE_COMPRESS_STATE;
			this->stage = STAGE_LEARN;
			this->stage_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else if (this->state == STATE_COMPRESS_STATE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter == 30000) {
			this->new_state_factor = 5;
		} else if (this->stage_iter == 60000) {
			this->new_state_factor = 1;
		}

		if (this->stage_iter >= 200000) {
			this->state = STATE_COMPRESS_STATE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_STATE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < 1.02*this->original_flat_error) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				// may be NULL
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				// update curr_fold without updating curr_scope_sizes
				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->curr_scope_input_folds[f_index];
						this->curr_scope_input_folds[f_index] = this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = NULL;
					}
				}

				if (this->compress_new_size == 0) {
					cout << "ending STATE_COMPRESS_STATE" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE" << endl;

					this->curr_scope_sizes = this->test_scope_sizes;
					this->curr_s_input_sizes = this->test_s_input_sizes;

					int obs_size;
					if (this->compound_actions[this->nodes.size()] == NULL) {
						obs_size = this->obs_sizes[this->nodes.size()];
					} else {
						obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
					}
					Node* new_node = new Node(this->id+"_n_"+to_string(this->nodes.size()),
											  (this->compound_actions[this->nodes.size()] != NULL),
											  this->action_input_input_layer,
											  this->action_input_input_sizes,
											  this->action_input_input_networks,
											  this->small_action_input_network,
											  this->action,
											  obs_size,
											  this->new_layer_size,
											  this->obs_network,
											  this->score_input_layer,
											  this->score_input_sizes,
											  this->score_input_networks,
											  this->small_score_network,
											  this->compress_num_layers,
											  this->compress_new_size,
											  this->input_layer,
											  this->input_sizes,
											  this->input_networks,
											  this->small_compression_network,
											  this->compressed_scope_sizes,
											  this->compressed_s_input_sizes);
					this->nodes.push_back(new_node);

					this->state = STATE_FINAL_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					cout << "ending STATE_COMPRESS_STATE" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_STATE" << endl;

					this->compress_size++;
					this->compress_new_size--;

					if (this->compress_new_size == 0) {
						this->test_scope_sizes.pop_back();
						this->test_s_input_sizes.pop_back();

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
							}
						}
					} else {
						this->test_scope_sizes.back()--;

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->curr_s_input_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compress_new_size);
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
								this->test_scope_input_folds[f_index]->add_scope(this->compress_new_size);
							}
						}
					}

					this->state = STATE_COMPRESS_STATE;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->new_state_factor = 25;
				}
			} else {
				if (this->test_compression_network != NULL) {
					delete this->test_compression_network;
					this->test_compression_network = NULL;
				}

				delete this->test_fold;
				this->test_fold = NULL;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = NULL;
					}
				}

				// undo previous increment
				if (this->compress_new_size == 0) {
					this->test_scope_sizes.push_back(1);
					this->test_s_input_sizes.push_back(0);
				} else {
					this->test_scope_sizes.back()++;
				}
				this->compress_size--;
				this->compress_new_size++;

				if (this->compress_size == 0) {
					cout << "ending STATE_COMPRESS_STATE" << endl;
					cout << "FAILURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE" << endl;

					this->compress_num_layers = 0;
					this->compress_new_size = 0;
					this->compressed_scope_sizes.clear();

					int obs_size;
					if (this->compound_actions[this->nodes.size()] == NULL) {
						obs_size = this->obs_sizes[this->nodes.size()];
					} else {
						obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
					}
					Node* new_node = new Node(this->id+"_n_"+to_string(this->nodes.size()),
											  (this->compound_actions[this->nodes.size()] != NULL),
											  this->action_input_input_layer,
											  this->action_input_input_sizes,
											  this->action_input_input_networks,
											  this->small_action_input_network,
											  this->action,
											  obs_size,
											  this->new_layer_size,
											  this->obs_network,
											  this->score_input_layer,
											  this->score_input_sizes,
											  this->score_input_networks,
											  this->small_score_network,
											  this->compress_num_layers,
											  this->compress_new_size,
											  this->input_layer,
											  this->input_sizes,
											  this->input_networks,
											  this->small_compression_network,
											  this->compressed_scope_sizes,
											  this->compressed_s_input_sizes);
					this->nodes.push_back(new_node);

					this->state = STATE_FINAL_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->compress_num_layers-1; sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[this->curr_scope_sizes.size()-1-sc_index];
					}
					if (this->compress_size > sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_STATE" << endl;
						cout << "FAILURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SMALL" << endl;

						this->curr_scope_sizes = this->test_scope_sizes;
						this->curr_s_input_sizes = this->test_s_input_sizes;

						int input_size = 0;
						for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
							input_size += this->compressed_scope_sizes[sc_index];
						}
						input_size += this->compressed_s_input_sizes[0];
						int hidden_size = 5*input_size;
						hidden_size = max(min(hidden_size, 50), 10);
						this->small_compression_network = new Network(input_size,
																	  hidden_size,
																	  this->compress_new_size);

						this->state = STATE_COMPRESS_SMALL;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else if (this->compress_size == sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_STATE" << endl;
						cout << "FAILURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE" << endl;

						this->compress_num_layers--;
						this->compress_new_size = 0;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_s_input_sizes.pop_back();
						this->test_s_input_sizes.push_back(this->compressed_s_input_sizes[0]);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
								this->test_scope_input_folds[f_index]->add_scope(this->compressed_scope_sizes[0]);
							}
						}

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());
						this->compressed_s_input_sizes.erase(this->compressed_s_input_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
						this->new_state_factor = 25;
					} else {
						cout << "ending STATE_COMPRESS_STATE" << endl;
						cout << "FAILURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE" << endl;

						this->compress_num_layers--;
						this->compress_new_size = sum_scope_sizes - this->compress_size;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_scope_sizes.push_back(this->compress_new_size);
						this->test_s_input_sizes.pop_back();
						this->test_s_input_sizes.push_back(this->compressed_s_input_sizes[0]);
						this->test_s_input_sizes.push_back(0);

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->curr_s_input_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						this->test_fold->add_scope(this->compress_new_size);
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
								this->test_scope_input_folds[f_index]->add_scope(this->compressed_scope_sizes[0]);
								this->test_scope_input_folds[f_index]->add_scope(this->compress_new_size);
							}
						}

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());
						this->compressed_s_input_sizes.erase(this->compressed_s_input_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
						this->new_state_factor = 25;
					}
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_SCOPE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter == 30000) {
			this->new_state_factor = 5;
		} else if (this->stage_iter == 60000) {
			this->new_state_factor = 1;
		}

		if (this->stage_iter >= 200000) {
			this->state = STATE_COMPRESS_SCOPE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_SCOPE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < 1.02*this->original_flat_error) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				// may be NULL
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				// update curr_fold without updating curr_scope_sizes
				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->curr_scope_input_folds[f_index];
						this->curr_scope_input_folds[f_index] = this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = NULL;
					}
				}

				if (this->compress_new_size == 0) {
					cout << "ending STATE_COMPRESS_SCOPE" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE" << endl;

					this->curr_scope_sizes = this->test_scope_sizes;
					this->curr_s_input_sizes = this->test_s_input_sizes;

					int obs_size;
					if (this->compound_actions[this->nodes.size()] == NULL) {
						obs_size = this->obs_sizes[this->nodes.size()];
					} else {
						obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
					}
					Node* new_node = new Node(this->id+"_n_"+to_string(this->nodes.size()),
											  (this->compound_actions[this->nodes.size()] != NULL),
											  this->action_input_input_layer,
											  this->action_input_input_sizes,
											  this->action_input_input_networks,
											  this->small_action_input_network,
											  this->action,
											  obs_size,
											  this->new_layer_size,
											  this->obs_network,
											  this->score_input_layer,
											  this->score_input_sizes,
											  this->score_input_networks,
											  this->small_score_network,
											  this->compress_num_layers,
											  this->compress_new_size,
											  this->input_layer,
											  this->input_sizes,
											  this->input_networks,
											  this->small_compression_network,
											  this->compressed_scope_sizes,
											  this->compressed_s_input_sizes);
					this->nodes.push_back(new_node);

					this->state = STATE_FINAL_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->compress_num_layers-1; sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[this->curr_scope_sizes.size()-1-sc_index];
					}
					if (this->compress_size > sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_SCOPE" << endl;
						cout << "SUCCESS" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_INPUT" << endl;

						// this->curr_scope_sizes > 1
						this->curr_scope_sizes = this->test_scope_sizes;
						this->curr_s_input_sizes = this->test_s_input_sizes;

						this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
						this->test_compression_network->fold_index++;

						this->state = STATE_COMPRESS_INPUT;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else if (this->compress_size == sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_SCOPE" << endl;
						cout << "SUCCESS" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE" << endl;

						this->compress_num_layers--;
						this->compress_new_size = 0;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_s_input_sizes.pop_back();
						this->test_s_input_sizes.push_back(this->compressed_s_input_sizes[0]);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
								this->test_scope_input_folds[f_index]->add_scope(this->compressed_scope_sizes[0]);
							}
						}

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());
						this->compressed_s_input_sizes.erase(this->compressed_s_input_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
						this->new_state_factor = 25;
					} else {
						cout << "ending STATE_COMPRESS_SCOPE" << endl;
						cout << "SUCCESS" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE" << endl;

						this->compress_num_layers--;
						this->compress_new_size = sum_scope_sizes - this->compress_size;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_scope_sizes.push_back(this->compress_new_size);
						this->test_s_input_sizes.pop_back();
						this->test_s_input_sizes.push_back(this->compressed_s_input_sizes[0]);
						this->test_s_input_sizes.push_back(0);

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->curr_s_input_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						this->test_fold->add_scope(this->compress_new_size);
						for (int f_index = 0; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->pop_scope();
								this->test_scope_input_folds[f_index]->add_scope(this->compressed_scope_sizes[0]);
								this->test_scope_input_folds[f_index]->add_scope(this->compress_new_size);
							}
						}

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());
						this->compressed_s_input_sizes.erase(this->compressed_s_input_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
						this->new_state_factor = 25;
					}
				}
			} else {
				if (this->test_compression_network != NULL) {
					delete this->test_compression_network;
					this->test_compression_network = NULL;
				}

				delete this->test_fold;
				this->test_fold = NULL;
				for (int f_index = 0; f_index < this->sequence_length; f_index++) {
					if (this->compound_actions[f_index] != NULL
							&& this->compound_actions[f_index]->num_inputs > 0) {
						delete this->test_scope_input_folds[f_index];
						this->test_scope_input_folds[f_index] = NULL;
					}
				}

				// undo previous uncompressed scope
				this->compress_num_layers++;
				if (this->compress_new_size > 0) {
					this->test_scope_sizes.pop_back();
					this->test_s_input_sizes.pop_back();
				}
				this->compress_new_size += this->test_scope_sizes.back();
				this->compressed_scope_sizes.insert(this->compressed_scope_sizes.begin(), this->test_scope_sizes.back());
				this->test_scope_sizes.pop_back();
				this->test_scope_sizes.push_back(this->compress_new_size);
				this->compressed_s_input_sizes.insert(this->compressed_s_input_sizes.begin(), this->test_s_input_sizes.back());
				this->test_s_input_sizes.pop_back();
				this->test_s_input_sizes.push_back(0);

				this->curr_scope_sizes = this->test_scope_sizes;
				this->curr_s_input_sizes = this->test_s_input_sizes;

				if (this->curr_scope_sizes.size() == 1) {
					cout << "ending STATE_COMPRESS_SCOPE" << endl;
					cout << "FAILURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_SMALL" << endl;

					int input_size = 0;
					for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
						input_size += this->compressed_scope_sizes[sc_index];
					}
					input_size += this->compressed_s_input_sizes[0];
					int hidden_size = 5*input_size;
					hidden_size = max(min(hidden_size, 50), 10);
					this->small_compression_network = new Network(input_size,
																  hidden_size,
																  this->compress_new_size);

					this->state = STATE_COMPRESS_SMALL;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_COMPRESS_SCOPE" << endl;
					cout << "FAILURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_INPUT" << endl;

					this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
					this->test_compression_network->fold_index++;

					this->state = STATE_COMPRESS_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_INPUT
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 200000) {
			this->state = STATE_COMPRESS_INPUT;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_INPUT
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < 0.0001
					|| (this->input_networks.size() > 0
						&& this->input_sizes.back() == (this->curr_scope_sizes[this->input_layer.back()]
							+ this->curr_s_input_sizes[this->input_layer.back()]))) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				if (this->curr_compression_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					cout << "ending STATE_COMPRESS_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_SMALL" << endl;

					int input_size = 0;
					for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
						input_size += this->compressed_scope_sizes[sc_index];
					}
					input_size += this->compressed_s_input_sizes[0];
					int hidden_size = 5*input_size;
					hidden_size = max(min(hidden_size, 50), 10);
					this->small_compression_network = new Network(input_size,
																  hidden_size,
																  this->compress_new_size);

					this->state = STATE_COMPRESS_SMALL;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_COMPRESS_INPUT" << endl;
					cout << "SUCCESS" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_INPUT" << endl;

					this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
					this->test_compression_network->fold_index++;

					// start from size 1 if have previous
					if (this->input_networks.size() > 0) {
						this->input_layer.push_back(this->test_compression_network->fold_index);
						this->input_sizes.push_back(1);
						int hidden_size = 5*(this->curr_scope_sizes[this->test_compression_network->fold_index]
							+ this->curr_s_input_sizes[this->test_compression_network->fold_index]);
						hidden_size = max(min(hidden_size, 50), 10);
						this->input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_compression_network->fold_index],
																		this->curr_s_input_sizes[this->test_compression_network->fold_index],
																		hidden_size,
																		1));
						this->test_compression_network->add_s_input(this->test_compression_network->fold_index+1,
																	1);
						if (this->test_compression_network->fold_index == (int)this->curr_s_input_sizes.size()-2) {
							this->compressed_s_input_sizes[0]++;
						} else {
							// add to curr_s_input_sizes permanently 1 at a time
							this->curr_s_input_sizes[this->test_compression_network->fold_index+1]++;
						}
					}

					this->state = STATE_COMPRESS_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				cout << "ending STATE_COMPRESS_INPUT" << endl;
				cout << "FAILURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_COMPRESS_INPUT" << endl;

				delete this->test_compression_network;
				this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
				this->test_compression_network->fold_index++;

				if (this->input_layer.size() > 0
						&& this->input_layer.back() == this->test_compression_network->fold_index) {
					this->input_sizes.back()++;
					delete this->input_networks.back();
					this->input_networks.pop_back();
				} else {
					this->input_layer.push_back(this->test_compression_network->fold_index);
					this->input_sizes.push_back(1);
				}
				int hidden_size = 5*(this->curr_scope_sizes[this->test_compression_network->fold_index]
					+ this->curr_s_input_sizes[this->test_compression_network->fold_index]);
				hidden_size = max(min(hidden_size, 50), 10);
				this->input_networks.push_back(new SmallNetwork(this->curr_scope_sizes[this->test_compression_network->fold_index],
																this->curr_s_input_sizes[this->test_compression_network->fold_index],
																hidden_size,
																this->input_sizes.back()));
				this->test_compression_network->add_s_input(this->test_compression_network->fold_index+1,
															this->input_sizes.back());
				if (this->test_compression_network->fold_index == (int)this->curr_s_input_sizes.size()-2) {
					this->compressed_s_input_sizes[0]++;
				} else {
					// add to curr_s_input_sizes permanently 1 at a time
					this->curr_s_input_sizes[this->test_compression_network->fold_index+1]++;
				}

				this->state = STATE_COMPRESS_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_SMALL) {
		if (this->stage_iter >= 200000) {
			cout << "ending STATE_COMPRESS_SMALL" << endl;
			cout << "starting STATE_FINAL_TUNE" << endl;

			delete this->curr_compression_network;

			int obs_size;
			if (this->compound_actions[this->nodes.size()] == NULL) {
				obs_size = this->obs_sizes[this->nodes.size()];
			} else {
				obs_size = this->compound_actions[this->nodes.size()]->num_outputs;
			}
			Node* new_node = new Node(this->id+"_n_"+to_string(this->nodes.size()),
									  (this->compound_actions[this->nodes.size()] != NULL),
									  this->action_input_input_layer,
									  this->action_input_input_sizes,
									  this->action_input_input_networks,
									  this->small_action_input_network,
									  this->action,
									  obs_size,
									  this->new_layer_size,
									  this->obs_network,
									  this->score_input_layer,
									  this->score_input_sizes,
									  this->score_input_networks,
									  this->small_score_network,
									  this->compress_num_layers,
									  this->compress_new_size,
									  this->input_layer,
									  this->input_sizes,
									  this->input_networks,
									  this->small_compression_network,
									  this->compressed_scope_sizes,
									  this->compressed_s_input_sizes);
			this->nodes.push_back(new_node);

			this->state = STATE_FINAL_TUNE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
			this->best_sum_error = -1.0;
		}
	} else if (this->state == STATE_FINAL_TUNE) {
		if (this->stage_iter >= 40000) {
			bool done = false;
			if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->stage_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try++;
				} else {
					done = true;
				}
			}

			if (done) {
				cout << "ending STATE_FINAL_TUNE" << endl;

				if ((int)this->nodes.size() == this->sequence_length) {
					cout << "DONE" << endl;

					this->state = STATE_DONE;
				} else {
					this->curr_action_input_network = NULL;
					this->test_action_input_network = NULL;
					this->action_input_input_layer.clear();
					this->action_input_input_sizes.clear();
					this->action_input_input_networks.clear();
					this->small_action_input_network = NULL;

					this->curr_score_network = NULL;
					this->test_score_network = NULL;
					this->score_input_layer.clear();
					this->score_input_sizes.clear();
					this->score_input_networks.clear();
					this->small_score_network = NULL;

					this->curr_compression_network = NULL;
					this->compress_size = 0;
					this->compress_num_layers = 0;
					this->compress_new_size = 0;
					this->compressed_scope_sizes.clear();
					this->compressed_s_input_sizes.clear();
					this->test_compression_network = NULL;
					this->input_layer.clear();
					this->input_sizes.clear();
					this->input_networks.clear();
					this->small_compression_network = NULL;

					if (this->compound_actions[this->nodes.size()] == NULL) {
						cout << "starting STATE_OBS" << endl;

						this->action = NULL;
						this->small_action_input_network = NULL;

						this->new_layer_size = 0;
						this->obs_network = NULL;

						this->test_scope_sizes = this->curr_scope_sizes;
						this->test_s_input_sizes = this->curr_s_input_sizes;

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->fold_index++;
						for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
							if (this->compound_actions[f_index] != NULL
									&& this->compound_actions[f_index]->num_inputs > 0) {
								this->test_scope_input_folds[f_index] = new FoldNetwork(this->curr_scope_input_folds[f_index]);
								this->test_scope_input_folds[f_index]->fold_index++;
							}
						}

						this->state = STATE_OBS;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
						this->new_state_factor = 25;
					} else if (this->compound_actions[this->nodes.size()] != NULL
							&& this->compound_actions[this->nodes.size()]->num_inputs > 0) {
						this->action = this->compound_actions[this->nodes.size()];
						if (this->curr_scope_sizes.size() == 0) {
							cout << "starting STATE_INNER_SCOPE_INPUT_SMALL" << endl;

							// edge case
							this->small_action_input_network = new SmallNetwork(0, 0, 0, this->compound_actions[this->nodes.size()]->num_inputs);

							this->state = STATE_INNER_SCOPE_INPUT_SMALL;
							this->stage_iter = 0;
							this->sum_error = 0.0;
						} else {
							cout << "starting STATE_INNER_SCOPE_INPUT" << endl;

							this->curr_action_input_network = new SubFoldNetwork(this->curr_scope_sizes,
																				 this->curr_s_input_sizes,
																				 this->compound_actions[this->nodes.size()]->num_inputs);

							this->state = STATE_INNER_SCOPE_INPUT;
							this->stage_iter = 0;
							this->sum_error = 0.0;
						}
					} else {
						// this->compound_actions[this->nodes.size()] != NULL && this->compound_actions[this->nodes.size()]->num_inputs == 0
						cout << "starting STATE_INNER_SCOPE" << endl;

						this->action = this->compound_actions[this->nodes.size()];
						this->small_action_input_network = NULL;

						this->state = STATE_INNER_SCOPE;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					}
				}
			}
		}
	}
}
