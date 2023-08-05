#include "action_node.h"

#include <iostream>

#include "constants.h"

using namespace std;

ActionNode::ActionNode(Scope* parent,
					   int id,
					   vector<int> target_indexes,
					   vector<StateNetwork*> state_networks,
					   ScoreNetwork* score_network,
					   int next_node_id) {
	this->type = NODE_TYPE_ACTION;

	this->parent = parent;
	this->id = id;

	this->target_indexes = target_indexes;
	this->state_networks = state_networks;
	this->score_network = score_network;
	this->misguess_network = new ScoreNetwork(this->score_network->state_size,
											  0,
											  20);
	this->next_node_id = next_node_id;

	this->is_explore = false;
	this->explore_best_experiment = NULL;
	this->experiment = NULL;
}

ActionNode::ActionNode(ActionNode* original,
					   Scope* parent,
					   int id,
					   int next_node_id) {
	this->type = NODE_TYPE_ACTION;

	this->parent = parent;
	this->id = id;

	this->target_indexes = original->target_indexes;
	for (int s_index = 0; s_index < (int)original->state_networks.size(); s_index++) {
		this->state_networks.push_back(new StateNetwork(original->state_networks[s_index]));
	}
	this->score_network = new ScoreNetwork(original->score_network);
	this->misguess_network = new ScoreNetwork(original->misguess_network);
	this->next_node_id = next_node_id;

	this->is_explore = false;
	this->explore_best_experiment = NULL;
	this->experiment = NULL;
}

ActionNode::ActionNode(ifstream& input_file,
					   Scope* parent,
					   int id) {
	this->type = NODE_TYPE_ACTION;

	this->parent = parent;
	this->id = id;

	string state_networks_size_line;
	getline(input_file, state_networks_size_line);
	int state_networks_size = stoi(state_networks_size_line);
	for (int s_index = 0; s_index < state_networks_size; s_index++) {
		string target_index_line;
		getline(input_file, target_index_line);
		this->target_indexes.push_back(stoi(target_index_line));

		ifstream state_network_save_file;
		state_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_state_" + to_string(s_index) + ".txt");
		this->state_networks.push_back(new StateNetwork(state_network_save_file));
		state_network_save_file.close();
	}

	ifstream score_network_save_file;
	score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_score.txt");
	this->score_network = new ScoreNetwork(score_network_save_file);
	score_network_save_file.close();

	ifstream misguess_network_save_file;
	misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_misguess.txt");
	this->misguess_network = new ScoreNetwork(misguess_network_save_file);
	misguess_network_save_file.close();

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	this->is_explore = false;
	this->explore_best_experiment = NULL;
	this->experiment = NULL;
}

ActionNode::~ActionNode() {
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}

	delete this->score_network;
	delete this->misguess_network;

	if (this->explore_best_experiment != NULL) {
		delete this->explore_best_experiment;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ActionNode::activate(vector<double>& flat_vals,
						  vector<ForwardContextLayer>& context,
						  vector<vector<StateNetwork*>>*& experiment_scope_state_networks,
						  vector<ScoreNetwork*>*& experiment_scope_score_networks,
						  int& experiment_scope_distance,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals.begin();
	history->starting_state_vals_snapshot = *(context.back().state_vals);

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		history->state_network_histories = vector<StateNetworkHistory*>(this->state_networks.size(), NULL);
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			if (context.back().states_initialized[this->target_indexes[s_index]]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->state_networks[s_index]);
				this->state_networks[s_index]->activate(history->obs_snapshot,
														history->starting_state_vals_snapshot,
														network_history);
				history->state_network_histories[s_index] = network_history;
				context.back().state_vals->at(this->target_indexes[s_index]) += this->state_networks[s_index]->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			if (context.back().states_initialized[this->target_indexes[s_index]]) {
				this->state_networks[s_index]->activate(history->obs_snapshot,
														history->starting_state_vals_snapshot);
				context.back().state_vals->at(this->target_indexes[s_index]) += this->state_networks[s_index]->output->acti_vals[0];
			}
		}
	}

	if (run_helper.explore_phase != EXPLORE_PHASE_EXPLORE) {
		history->ending_state_vals_snapshot = *(context.back().state_vals);

		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
		this->score_network->activate(history->ending_state_vals_snapshot,
									  score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_output = this->score_network->output->acti_vals[0];
		run_helper.predicted_score += run_helper.scale_factor*this->score_network->output->acti_vals[0];
	}
	// don't actually need to save history during EXPLORE_PHASE_MEASURE, but won't special case for now

	flat_vals.erase(flat_vals.begin());

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (run_helper.experiment->state == EXPLORE_PHASE_EXPERIMENT) {
			if (experiment_scope_state_networks->at(this->id).size() == 0) {
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					experiment_scope_state_networks->at(this->id).push_back(
						new StateNetwork(this->parent->num_states,
										 NUM_NEW_STATES,
										 0,
										 20));
					experiment_scope_state_networks->at(this->id).back()->update_lasso_weights(experiment_scope_distance);
				}
				experiment_scope_score_networks->at(this->id) = new ScoreNetwork(this->parent->num_states,
																				 NUM_NEW_STATES,
																				 20);
				experiment_scope_score_networks->at(this->id)->update_lasso_weights(experiment_scope_distance);
			}
		}

		if (experiment_scope_state_networks != NULL
				&& this->id < experiment_scope_state_networks->size()
				&& experiment_scope_state_networks->at(this->id).size() != 0) {
			history->starting_new_state_vals_snapshot = run_helper.new_state_vals;

			history->new_state_network_histories = vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL);
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
					// do nothing
				} else if (experiment_scope_state_networks->at(this->id)[s_index] == NULL) {
					// do nothing
				} else {
					StateNetwork* network = experiment_scope_state_networks->at(this->id)[s_index];
					StateNetworkHistory* network_history = new StateNetworkHistory(network);
					network->new_activate(history->obs_snapshot,
										  history->starting_state_vals_snapshot,
										  history->starting_new_state_vals_snapshot,
										  network_history);
					history->new_state_network_histories[s_index] = network_history;
					run_helper.new_state_vals[s_index] += network->output->acti_vals[0];
				}
			}

			history->ending_new_state_vals_snapshot = run_helper.new_state_vals;

			ScoreNetwork* score_network = experiment_scope_score_networks->at(this->id);
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
			score_network->new_activate(history->ending_state_vals_snapshot,
										history->ending_new_state_vals_snapshot,
										score_network_history);
			history->new_score_network_history = score_network_history;
			history->new_score_network_output = score_network->output->acti_vals[0];

			run_helper.predicted_score += run_helper.scale_factor*score_network->output->acti_vals[0];
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		if (run_helper.experiment->state_iter < 100000) {
			if (experiment_scope_score_networks != NULL
					&& this->id < experiment_scope_score_networks->size()
					&& experiment_scope_score_networks->at(this->id) != NULL) {
				ScoreNetwork* score_network = experiment_scope_score_networks->at(this->id);
				score_network->activate(history->ending_state_vals_snapshot);
				history->new_score_network_output = score_network->output->acti_vals[0];

				double temp_score_scale = (100000.0-run_helper.experiment->state_iter)/100000.0;
				run_helper.predicted_score += run_helper.scale_factor*temp_score_scale*score_network->output->acti_vals[0];
			}
		}
	}
}

void ActionNode::backprop(vector<BackwardContextLayer>& context,
						  double& scale_factor_error,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
		vector<double> new_state_errors_snapshot = run_helper.new_state_errors;
		for (int s_index = 0; s_index < (int)history->new_state_network_histories.size(); s_index++) {
			if (history->new_state_network_histories[s_index] != NULL) {
				StateNetwork* network = history->new_state_network_histories[s_index]->network;
				double new_state_network_target_max_update;
				if (run_helper.experiment->state_iter <= 100000) {
					new_state_network_target_max_update = 0.05;
				} else {
					new_state_network_target_max_update = 0.01;
				}
				if (run_helper.experiment->state_iter <= 20000) {
					network->new_backprop(new_state_errors_snapshot[s_index],
										  run_helper.new_state_errors,
										  new_state_network_target_max_update,
										  history->obs_snapshot,
										  history->starting_state_vals_snapshot,
										  history->starting_new_state_vals_snapshot,
										  history->new_state_network_histories[s_index]);
				} else {
					network->new_lasso_backprop(new_state_errors_snapshot[s_index],
												run_helper.new_state_errors,
												new_state_network_target_max_update,
												history->obs_snapshot,
												history->starting_state_vals_snapshot,
												history->starting_new_state_vals_snapshot,
												history->new_state_network_histories[s_index]);
				}
			}
		}
		for (int st_index = 0; st_index < (int)history->experiment_sequence_step_indexes.size(); st_index++) {
			for (int i_index = 0; i_index < (int)history->input_state_network_histories[st_index].size(); i_index++) {
				if (history->input_state_network_histories[st_index][i_index] != NULL) {
					StateNetwork* network = history->input_state_network_histories[st_index][i_index]->network;
					double new_input_network_target_max_update;
					if (run_helper.experiment->state_iter <= 100000) {
						new_input_network_target_max_update = 0.05;
					} else {
						new_input_network_target_max_update = 0.01;
					}
					if (run_helper.experiment->state_iter <= 20000) {
						network->new_backprop(run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
											  run_helper.new_state_errors,
											  run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
											  new_input_network_target_max_update,
											  history->obs_snapshot,
											  history->starting_state_vals_snapshot,
											  history->starting_new_state_vals_snapshot,
											  history->input_vals_snapshots[st_index][i_index],
											  history->input_state_network_histories[st_index][i_index]);
					} else {
						network->new_lasso_backprop(run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
													run_helper.new_state_errors,
													run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
													new_input_network_target_max_update,
													history->obs_snapshot,
													history->starting_state_vals_snapshot,
													history->starting_new_state_vals_snapshot,
													history->input_vals_snapshots[st_index][i_index],
													history->input_state_network_histories[st_index][i_index]);
					}
				}
			}
		}

		double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

		// don't factor in new_score_network in scale_factor_error

		ScoreNetwork* new_score_network = history->new_score_network_history->network;
		double new_score_network_target_max_update;
		if (run_helper.experiment->state_iter <= 100000) {
			new_score_network_target_max_update = 0.05;
		} else {
			new_score_network_target_max_update = 0.01;
		}
		if (run_helper.experiment->state_iter <= 20000) {
			new_score_network->new_backprop(run_helper.scale_factor*predicted_score_error,
											run_helper.new_state_errors,
											new_score_network_target_max_update,
											history->ending_state_vals_snapshot,
											history->ending_new_state_vals_snapshot,
											history->new_score_network_history);
		} else {
			new_score_network->new_lasso_backprop(run_helper.scale_factor*predicted_score_error,
												  run_helper.new_state_errors,
												  new_score_network_target_max_update,
												  history->ending_state_vals_snapshot,
												  history->ending_new_state_vals_snapshot,
												  history->new_score_network_history);
		}

		run_helper.predicted_score -= run_helper.scale_factor*history->new_score_network_output;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		vector<double> new_state_errors_snapshot = run_helper.new_state_errors;
		for (int s_index = 0; s_index < (int)history->new_state_network_histories.size(); s_index++) {
			if (history->new_state_network_histories[s_index] != NULL) {
				StateNetwork* network = history->new_state_network_histories[s_index]->network;
				network->new_backprop(new_state_errors_snapshot[this->target_indexes[s_index]],
									  run_helper.new_state_errors,
									  0.01,
									  history->obs_snapshot,
									  history->starting_state_vals_snapshot,
									  history->starting_new_state_vals_snapshot,
									  history->new_state_network_histories[s_index]);
			}
		}
		if (run_helper.experiment->state == EXPERIMENT_STATE_FIRST_CLEAN) {
			for (int st_index = 0; st_index < (int)history->experiment_sequence_step_indexes.size(); st_index++) {
				for (int i_index = 0; i_index < (int)history->input_state_network_histories[st_index].size(); i_index++) {
					if (history->input_state_network_histories[st_index][i_index] != NULL) {
						StateNetwork* network = history->input_state_network_histories[st_index][i_index]->network;
						network->new_lasso_backprop(run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
													run_helper.new_state_errors,
													run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
													0.01,
													history->obs_snapshot,
													history->starting_state_vals_snapshot,
													history->starting_new_state_vals_snapshot,
													history->input_vals_snapshots[st_index][i_index],
													history->input_state_network_histories[st_index][i_index]);
					}
				}
			}
		} else {
			for (int st_index = 0; st_index < (int)history->experiment_sequence_step_indexes.size(); st_index++) {
				for (int i_index = 0; i_index < (int)history->input_state_network_histories[st_index].size(); i_index++) {
					if (history->input_state_network_histories[st_index][i_index] != NULL) {
						StateNetwork* network = history->input_state_network_histories[st_index][i_index]->network;
						network->new_backprop(run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
											  run_helper.new_state_errors,
											  run_helper.new_input_errors[history->experiment_sequence_step_indexes[st_index]][i_index],
											  0.01,
											  history->obs_snapshot,
											  history->starting_state_vals_snapshot,
											  history->starting_new_state_vals_snapshot,
											  history->input_vals_snapshots[st_index][i_index],
											  history->input_state_network_histories[st_index][i_index]);
					}
				}
			}
		}

		if (history->new_score_network_history != NULL) {
			double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

			ScoreNetwork* new_score_network = history->new_score_network_history->network;
			new_score_network->new_backprop(run_helper.scale_factor*predicted_score_error,
											run_helper.new_state_errors,
											0.01,
											history->ending_state_vals_snapshot,
											history->ending_new_state_vals_snapshot,
											history->new_score_network_history);

			run_helper.predicted_score -= run_helper.scale_factor*history->new_score_network_output;
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		if (run_helper.experiment->state_iter < 100000) {
			double temp_score_scale = (100000.0-run_helper.experiment->state_iter)/100000.0;
			run_helper.predicted_score -= run_helper.scale_factor*temp_score_scale*history->new_score_network_output;
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

		scale_factor_error += history->score_network_update*predicted_score_error;

		this->score_network->backprop_weights_with_no_error_signal(
			run_helper.scale_factor*predicted_score_error,
			0.002,
			history->ending_state_vals_snapshot,
			history->score_network_history);

		run_helper.predicted_score -= run_helper.scale_factor*history->score_network_output;

		this->misguess_network->activate(history->ending_state_vals_snapshot);
		double misguess_error = run_helper.final_misguess - this->misguess_network->output->acti_vals[0];
		this->misguess_network->backprop_weights_with_no_error_signal(misguess_error, 0.002);
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (!run_helper.backprop_is_pre_experiment) {
			double predicted_score_error = run_helper.target_val - run_helper.predicted_score;
			this->score_network->backprop_errors_with_no_weight_change(
				run_helper.scale_factor*predicted_score_error,
				*(context.back().state_errors),
				history->ending_state_vals_snapshot,
				history->score_network_history);
		}

		run_helper.predicted_score -= run_helper.scale_factor*history->score_network_output;
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (!run_helper.backprop_is_pre_experiment) {
			vector<double> state_errors_snapshot = *(context.back().state_errors);

			for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
				if (history->state_network_histories[s_index] != NULL) {
					this->state_networks[s_index]->backprop_errors_with_no_weight_change(
						state_errors_snapshot[this->target_indexes[s_index]],
						*(context.back().state_errors),
						history->obs_snapshot,
						history->starting_state_vals_snapshot,
						history->state_network_histories[s_index]);
				}
			}
		}
	}
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		output_file << this->target_indexes[s_index] << endl;

		ofstream state_network_save_file;
		state_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_state_" + to_string(s_index) + ".txt");
		this->state_networks[s_index]->save(state_network_save_file);
		state_network_save_file.close();
	}

	ofstream score_network_save_file;
	score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_score.txt");
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	ofstream misguess_network_save_file;
	misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_misguess.txt");
	this->misguess_network->save(misguess_network_save_file);
	misguess_network_save_file.close();

	output_file << this->next_node_id << endl;
}

void ActionNode::save_for_display(ofstream& output_file) {

}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;

	this->score_network_history = NULL;

	this->experiment_history = NULL;

	this->new_score_network_history = NULL;
	this->new_score_network_output = 0.0;	// initialize to 0.0 to make some EXPLORE_PHASE_WRAPUP logic easier
}

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->node = original->node;

	this->obs_snapshot = original->obs_snapshot;
	this->starting_state_vals_snapshot = original->starting_state_vals_snapshot;
	this->ending_state_vals_snapshot = original->ending_state_vals_snapshot;

	this->score_network_history = NULL;

	this->experiment_history = NULL;

	this->new_score_network_history = NULL;
}

ActionNodeHistory::~ActionNodeHistory() {
	for (int s_index = 0; s_index < (int)this->state_network_histories.size(); s_index++) {
		if (this->state_network_histories[s_index] != NULL) {
			delete this->state_network_histories[s_index];
		}
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}

	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}

	for (int s_index = 0; s_index < (int)this->new_state_network_histories.size(); s_index++) {
		if (this->new_state_network_histories[s_index] != NULL) {
			delete this->new_state_network_histories[s_index];
		}
	}

	if (this->new_score_network_history != NULL) {
		delete this->new_score_network_history;
	}

	for (int s_index = 0; s_index < (int)this->input_state_network_histories.size(); s_index++) {
		for (int i_index = 0; i_index < (int)this->input_state_network_histories[s_index].size(); i_index++) {
			if (this->input_state_network_histories[s_index][i_index] != NULL) {
				delete this->input_state_network_histories[s_index][i_index];
			}
		}
	}
}
