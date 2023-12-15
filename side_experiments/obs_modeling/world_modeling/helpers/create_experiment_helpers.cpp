#include "helpers.h"

#include <iostream>

#include "connection_experiment.h"
#include "experiment.h"
#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

void create_commit(vector<WorldState*>& state_history,
				   vector<int>& sequence_index_history,
				   vector<Action*>& action_sequence,
				   vector<vector<int>>& action_state_sequence,
				   int h_index,
				   int experiment_length,
				   vector<WorldState*>& experiment_states) {
	for (int l_index = 0; l_index < experiment_length; l_index++) {
		experiment_states.push_back(new WorldState());
		experiment_states.back()->default_transition = experiment_states.back();
	}

	if (experiment_length > 1) {
		int l_index = 0;
		int curr_h_index = h_index;
		int curr_sequence_index = sequence_index_history[h_index];
		while (true) {
			if (sequence_index_history[curr_h_index+1] > curr_sequence_index) {
				experiment_states[l_index]->action_transitions.push_back(experiment_states[l_index+1]);
				experiment_states[l_index]->action_transition_actions.push_back(action_sequence[curr_sequence_index+1]);

				vector<pair<int, int>> action_states;

				vector<pair<int, int>> possible_assignments;
				for (int s_index = 0; s_index < (int)action_state_sequence[curr_sequence_index+1].size(); s_index++) {
					if (action_state_sequence[curr_sequence_index+1][s_index] != -1) {
						possible_assignments.push_back({s_index, action_state_sequence[curr_sequence_index+1][s_index]});
					}
				}

				geometric_distribution<int> num_assignments_distribution(0.5);
				int num_assignments = num_assignments_distribution(generator);
				if (num_assignments > (int)possible_assignments.size()) {
					num_assignments = (int)possible_assignments.size();
				}

				for (int a_index = 0; a_index < num_assignments; a_index++) {
					uniform_int_distribution<int> assignment_distribution(0, possible_assignments.size()-1);
					int index = assignment_distribution(generator);
					action_states.push_back(possible_assignments[index]);
					possible_assignments.erase(possible_assignments.begin() + index);
				}

				experiment_states[l_index]->action_transition_states.push_back(action_states);

				l_index++;
				curr_sequence_index = sequence_index_history[curr_h_index+1];
				if (l_index >= experiment_length-1) {
					break;
				}
			}

			curr_h_index++;
		}
	}
}

void create_cancel(vector<WorldState*>& state_history,
				   vector<int>& sequence_index_history,
				   vector<Action*>& action_sequence,
				   vector<vector<int>>& action_state_sequence,
				   int h_index,
				   int experiment_length,
				   vector<WorldState*>& experiment_states) {
	for (int l_index = 0; l_index < experiment_length; l_index++) {
		experiment_states.push_back(new WorldState());
	}

	if (experiment_length > 1) {
		int l_index = 0;
		int curr_h_index = h_index;
		int curr_sequence_index = sequence_index_history[h_index];
		while (true) {
			if (sequence_index_history[curr_h_index+1] > curr_sequence_index) {
				/**
				 * - simply insert new transition earliest so takes priority
				 */
				experiment_states[l_index]->action_transitions.push_back(experiment_states[l_index+1]);
				experiment_states[l_index]->action_transition_actions.push_back(action_sequence[curr_sequence_index+1]);

				vector<pair<int, int>> action_states;

				vector<pair<int, int>> possible_assignments;
				for (int s_index = 0; s_index < (int)action_state_sequence[curr_sequence_index+1].size(); s_index++) {
					if (action_state_sequence[curr_sequence_index+1][s_index] != -1) {
						possible_assignments.push_back({s_index, action_state_sequence[curr_sequence_index+1][s_index]});
					}
				}

				geometric_distribution<int> num_assignments_distribution(0.5);
				int num_assignments = num_assignments_distribution(generator);
				if (num_assignments > (int)possible_assignments.size()) {
					num_assignments = (int)possible_assignments.size();
				}

				for (int a_index = 0; a_index < num_assignments; a_index++) {
					uniform_int_distribution<int> assignment_distribution(0, possible_assignments.size()-1);
					int index = assignment_distribution(generator);
					action_states.push_back(possible_assignments[index]);
					possible_assignments.erase(possible_assignments.begin() + index);
				}

				experiment_states[l_index]->action_transition_states.push_back(action_states);

				WorldState* original_state = state_history[curr_h_index+1];
				/**
				 * - don't copy obs_transitions so will take experiment path
				 *   - obs/state averages might be different anyways because new spot
				 */
				experiment_states[l_index]->action_transitions = original_state->action_transitions;
				experiment_states[l_index]->action_transition_actions = original_state->action_transition_actions;
				experiment_states[l_index]->action_transition_states = original_state->action_transition_states;
				experiment_states[l_index]->default_transition = original_state->default_transition;

				l_index++;
				curr_sequence_index = sequence_index_history[curr_h_index+1];
				if (l_index >= experiment_length-1) {
					break;
				}
			}

			curr_h_index++;
		}
	}

	experiment_states.back()->default_transition = experiment_states.back();
}

void create_experiment(vector<WorldState*>& state_history,
					   vector<int>& sequence_index_history,
					   vector<Action*>& action_sequence,
					   vector<vector<int>>& action_state_sequence) {
	uniform_int_distribution<int> history_distribution(0, state_history.size()-1);
	int h_index = history_distribution(generator);
	int sequence_index = sequence_index_history[h_index];

	uniform_int_distribution<int> is_bool_distribution(0, 1);
	bool is_obs = is_bool_distribution(generator) == 0;
	int obs_index;
	bool obs_is_greater;
	Action* action;
	vector<pair<int, int>> action_states;
	if (is_obs) {
		uniform_int_distribution<int> obs_index_distribution(0, world_model->num_states);
		obs_index = obs_index_distribution(generator) - 1;
		uniform_int_distribution<int> is_greater_distribution(0, 1);
		obs_is_greater = is_greater_distribution(generator);
		action = NULL;
		/**
		 * - simply add even if duplicate
		 *   - will take effect when previous fails
		 */
	} else {
		obs_index = -1;
		obs_is_greater = false;
		action = action_sequence[sequence_index];

		map<Action*, pair<vector<pair<int, int>>, AbstractExperiment*>>::iterator it =
			state_history[h_index]->action_experiments.find(action);
		if (it != state_history[h_index]->action_experiments.end()) {
			return;
		}
		/**
		 * - simply abandon if already occupied
		 *   - otherwise, will need extra bookkeeping and may lead to too many experiments at once
		 */

		vector<pair<int, int>> possible_assignments;
		for (int s_index = 0; s_index < (int)action_state_sequence[sequence_index].size(); s_index++) {
			if (action_state_sequence[sequence_index][s_index] != -1) {
				possible_assignments.push_back({s_index, action_state_sequence[sequence_index][s_index]});
			}
		}

		geometric_distribution<int> num_assignments_distribution(0.5);
		int num_assignments = num_assignments_distribution(generator);
		if (num_assignments > (int)possible_assignments.size()) {
			num_assignments = (int)possible_assignments.size();
		}

		for (int a_index = 0; a_index < num_assignments; a_index++) {
			uniform_int_distribution<int> assignment_distribution(0, possible_assignments.size()-1);
			int index = assignment_distribution(generator);
			action_states.push_back(possible_assignments[index]);
			possible_assignments.erase(possible_assignments.begin() + index);
		}
	}

	AbstractExperiment* experiment;
	uniform_int_distribution<int> type_distribution(0, 1);
	if (type_distribution(generator) == 0) {
		geometric_distribution<int> length_distribution(0.5);
		int experiment_length = 1 + length_distribution(generator);
		if (sequence_index + experiment_length > (int)action_sequence.size()) {
			experiment_length = (int)action_sequence.size() - sequence_index;
		}

		vector<WorldState*> experiment_states;

		uniform_int_distribution<int> experiment_type_distribution(0, 1);
		if (experiment_type_distribution(generator) == 0) {
			create_commit(state_history,
						  sequence_index_history,
						  action_sequence,
						  action_state_sequence,
						  h_index,
						  experiment_length,
						  experiment_states);
		} else {
			create_cancel(state_history,
						  sequence_index_history,
						  action_sequence,
						  action_state_sequence,
						  h_index,
						  experiment_length,
						  experiment_states);
		}

		experiment = new Experiment(state_history[h_index],
									is_obs,
									obs_index,
									obs_is_greater,
									action,
									action_states,
									experiment_states);
	} else {
		uniform_int_distribution<int> state_distribution(0, world_model->world_states.size()-1);
		experiment = new ConnectionExperiment(state_history[h_index],
											  is_obs,
											  obs_index,
											  obs_is_greater,
											  action,
											  action_states,
											  world_model->world_states[state_distribution(generator)]);
	}

	if (is_obs) {
		state_history[h_index]->obs_experiments.push_back(experiment);
		state_history[h_index]->obs_experiment_indexes.push_back(obs_index);
		state_history[h_index]->obs_experiment_is_greater.push_back(obs_is_greater);
	} else {
		state_history[h_index]->action_experiments[action] = {action_states, experiment};
	}
}
