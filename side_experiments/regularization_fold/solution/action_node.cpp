#include "action_node.h"

using namespace std;



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
		new_score_network->new_backprop(run_helper.scale_factor*predicted_score_error,
										run_helper.new_state_errors,
										new_score_network_target_max_update,
										history->ending_state_vals_snapshot,
										history->ending_new_state_vals_snapshot,
										history->new_score_network_history);

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

	vector<double> state_errors_snapshot = *(context.back().state_errors);

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		this->average_score = 0.9999*this->average_score + 0.0001*run_helper.target_val;
		double curr_score_variance = (this->average_score - run_helper.target_val)*(this->average_score - run_helper.target_val);
		this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

		this->average_misguess = 0.9999*this->average_misguess + 0.0001*run_helper.final_misguess;
		double curr_misguess_variance = (this->average_misguess - run_helper.final_misguess)*(this->average_misguess - run_helper.final_misguess);
		this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

		this->average_impact = 0.9999*this->average_impact + 0.0001*abs(scale_factor*history->score_network_update);

		double predicted_score_error = run_helper.target_val - run_helper.predicted_score;

		scale_factor_error += history->score_network_update*predicted_score_error;

		this->score_network->backprop_weights_with_no_error_signal(
			run_helper.scale_factor*predicted_score_error,
			0.002,
			history->ending_state_vals_snapshot,
			history->score_network_history);

		run_helper.predicted_score -= run_helper.scale_factor*history->score_network_output;
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



ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;



	this->new_score_network_history = NULL;
	this->new_score_network_output = 0.0;
}
