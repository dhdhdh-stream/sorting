#include "world_state.h"

using namespace std;



void WorldState::activate(WorldState*& curr_state,
						  int& curr_sequence_index,
						  vector<double>& obs_sequence,
						  vector<Action*>& action_sequence,
						  vector<vector<int>>& action_state_sequence,
						  vector<vector<double>>& state_vals_sequence,
						  RunHelper& run_helper) {
	/**
	 * - use curr obs, but past state_vals
	 */
	for (int t_index = 0; t_index < (int)this->obs_transitions.size(); t_index++) {
		if (this->obs_indexes[t_index] == -1) {
			if (this->obs_is_greater[t_index]) {
				if (obs_sequence[curr_sequence_index] > this->obs_average) {
					curr_state = this->obs_transitions[t_index];
					return;
				}
			} else {
				if (obs_sequence[curr_sequence_index] < this->obs_average) {
					curr_state = this->obs_transitions[t_index];
					return;
				}
			}
		} else {
			if (curr_sequence_index > 0) {
				if (this->obs_is_greater[t_index]) {
					if (state_vals_sequence[curr_sequence_index-1][this->obs_indexes[t_index]] > this->state_averages[this->obs_indexes[t_index]]) {
						curr_state = this->obs_transitions[t_index];
						return;
					}
				} else {
					if (state_vals_sequence[curr_sequence_index-1][this->obs_indexes[t_index]] < this->state_averages[this->obs_indexes[t_index]]) {
						curr_state = this->obs_transitions[t_index];
						return;
					}
				}
			}
		}
	}

	for (int e_index = 0; e_index < (int)this->obs_experiments.size(); e_index++) {
		bool is_match = false;
		if (this->obs_experiments_indexes[t_index] == -1) {
			if (this->obs_experiments_is_greater[t_index]) {
				if (obs_sequence[curr_sequence_index] > this->obs_average) {
					is_match = true;
				}
			} else {
				if (obs_sequence[curr_sequence_index] < this->obs_average) {
					is_match = true;
				}
			}
		} else {
			if (curr_sequence_index > 0) {
				if (this->obs_experiments_is_greater[t_index]) {
					if (state_vals_sequence[curr_sequence_index-1][this->obs_indexes[t_index]] > this->state_averages[this->obs_indexes[t_index]]) {
						is_match = true;
					}
				} else {
					if (state_vals_sequence[curr_sequence_index-1][this->obs_indexes[t_index]] < this->state_averages[this->obs_indexes[t_index]]) {
						is_match = true;
					}
				}
			}
		}

		if (is_match) {
			bool is_selected = this->obs_experiments[e_index]->activate(
				curr_state,
				run_helper);

			if (is_selected) {
				return;
			}
		}
	}

	if (this->experiment_hook != NULL) {
		this->hook_obs.push_back(obs_sequence[curr_sequence_index]);
		if (curr_sequence_index > 0) {
			this->hook_state_vals.push_back(vector<double>(world_model->num_states, 0.0));
		} else {
			this->hook_state_vals.push_back(state_vals_sequence[curr_sequence_index-1]);
		}
	}

	curr_sequence_index++;

	int matching_index = -1;
	for (int t_index = 0; t_index < (int)this->action_transitions.size(); t_index++) {
		if (this->action_transition_actions[t_index] == action_sequence[curr_sequence_index]) {
			bool is_match = true;
			for (int s_index = 0; s_index < (int)this->action_transition_states[t_index].size(); s_index++) {
				pair<int, int> p = this->action_transition_states[t_index][s_index];
				if (action_state_sequence[curr_sequence_index][p.first] != p.second) {
					is_match = false;
					break;
				}
			}

			if (is_match) {
				matching_index = t_index;
				break;
			}
		}
	}

	if (matching_index != -1) {
		curr_state = this->action_transitions[matching_index];
	} else {
		curr_state = this->default_transition;
	}

	map<Action*, pair<vector<pair<int, int>>, AbstractExperiment*>>::iterator it = this->action_experiments
		.find(action_sequence[curr_sequence_index]);
	if (it != this->action_experiments.end()) {
		bool is_match = true;
		for (int s_index = 0; s_index < (int)it->second.first.size(); s_index++) {
			pair<int, int> p = it->second.first[s_index];
			if (action_state_sequence[curr_sequence_index][p.first] != p.second) {
				is_match = false;
				break;
			}
		}

		if (is_match) {
			it->second.second->activate(curr_state,
										run_helper);
		}
	}
}

void WorldState::measure_activate() {

}

void WorldState::generate() {
	// {
	// 	map<Action*, vector<Transform*>>::iterator it = this->action_impacts.find(curr_action);
	// 	for (int s_index = 0; s_index < (int)curr_action_state.size(); s_index++) {
	// 		if (curr_action_state[s_index] != -1) {
	// 			double curr_state_val = state_vals[curr_action_state[s_index]];
	// 			double new_state_val = it->second[s_index]->activate(curr_state_val);
	// 			state_vals[curr_action_state[s_index]] = new_state_val;
	// 		}
	// 	}
	// }
}
