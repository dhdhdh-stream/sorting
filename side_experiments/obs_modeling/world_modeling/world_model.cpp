#include "world_model.h"

#include "abstract_experiment.h"
#include "helpers.h"
#include "world_state.h"

using namespace std;

WorldModel::WorldModel() {
	// do nothing
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->world_states.size(); s_index++) {
		delete this->world_states[s_index];
	}
}

void WorldModel::init() {
	// TODO: handle states
	this->num_states = 0;

	WorldState* world_state = new WorldState();
	world_state->id = 0;
	world_state->val_average = 0.0;
	world_state->state_val_impacts = vector<double>(this->num_states, 0.0);
	world_state->obs_average = 0.0;
	world_state->state_obs_impacts = vector<double>(this->num_states, 0.0);
	world_state->state_averages = vector<double>(this->num_states, 0.0);
	world_state->default_transition = world_state;

	this->world_states.push_back(world_state);
}

bool WorldModel::activate(vector<double>& obs_sequence,
						  vector<Action*>& action_sequence,
						  vector<vector<int>>& action_state_sequence,
						  vector<vector<double>>& state_vals_sequence,
						  double target_val) {
	RunHelper run_helper;

	vector<WorldState*> state_history;
	vector<int> sequence_index_history;

	WorldState* curr_state = this->world_states[0];
	int curr_sequence_index = 0;
	set<WorldState*> repeat_tracker;
	while (curr_sequence_index < (int)obs_sequence.size()) {
		state_history.push_back(curr_state);
		sequence_index_history.push_back(curr_sequence_index);
		curr_state->activate(curr_state,
							 curr_sequence_index,
							 obs_sequence,
							 action_sequence,
							 action_state_sequence,
							 state_vals_sequence,
							 repeat_tracker,
							 run_helper);
	}
	// don't need to add ending

	if (run_helper.selected_experiment != NULL) {
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
			experiment->average_remaining_experiments_from_start =
				0.9 * experiment->average_remaining_experiments_from_start
				+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
					+ run_helper.selected_experiment->average_remaining_experiments_from_start);
		}

		run_helper.selected_experiment->backprop(target_val,
												 curr_state,
												 state_vals_sequence.back());

		if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			for (int h_index = 0; h_index < (int)this->world_states.size(); h_index++) {
				this->world_states[h_index]->success_reset();
			}

			ofstream output_file;
			output_file.open("display.txt");
			save_for_display(output_file);
			output_file.close();

			return true;
		} else if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_FAIL) {
			if (run_helper.selected_experiment->is_obs) {
				int experiment_index;
				for (int e_index = 0; e_index < (int)run_helper.selected_experiment->parent->obs_experiments.size(); e_index++) {
					if (run_helper.selected_experiment->parent->obs_experiments[e_index] == run_helper.selected_experiment) {
						experiment_index = e_index;
						break;
					}
				}

				run_helper.selected_experiment->parent->obs_experiments.erase(
					run_helper.selected_experiment->parent->obs_experiments.begin() + experiment_index);
				run_helper.selected_experiment->parent->obs_experiment_indexes.erase(
					run_helper.selected_experiment->parent->obs_experiment_indexes.begin() + experiment_index);
				run_helper.selected_experiment->parent->obs_experiment_is_greater.erase(
					run_helper.selected_experiment->parent->obs_experiment_is_greater.begin() + experiment_index);
			} else {
				run_helper.selected_experiment->parent->action_experiments
					.erase(run_helper.selected_experiment->action);
			}

			delete run_helper.selected_experiment;
		}
	} else {
		if (run_helper.experiments_seen.size() == 0) {
			create_experiment(state_history,
							  sequence_index_history,
							  action_sequence,
							  action_state_sequence);
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}
		}
	}

	return false;
}

double WorldModel::measure_activate(vector<double>& obs_sequence,
									vector<Action*>& action_sequence,
									vector<vector<int>>& action_state_sequence,
									vector<vector<double>>& state_vals_sequence,
									double target_val) {
	WorldState* curr_state = this->world_states[0];
	int curr_sequence_index = 0;
	set<WorldState*> repeat_tracker;
	while (curr_sequence_index < (int)obs_sequence.size()) {
		curr_state->measure_activate(curr_state,
									 curr_sequence_index,
									 obs_sequence,
									 action_sequence,
									 action_state_sequence,
									 state_vals_sequence,
									 repeat_tracker);
	}

	double predicted_score = curr_state->val_average;
	// for (int s_index = 0; s_index < this->num_states; s_index++) {
	// 	predicted_score += curr_state->state_val_impacts[s_index] * state_vals_sequence.back()[s_index];
	// }

	double misguess = (target_val - predicted_score)*(target_val - predicted_score);
	return misguess;
}

void WorldModel::generate() {

}

void WorldModel::save_for_display(ofstream& output_file) {
	output_file << this->world_states.size() << endl;
	for (int s_index = 0; s_index < (int)this->world_states.size(); s_index++) {
		output_file << s_index << endl;
		this->world_states[s_index]->save_for_display(output_file);
	}
}
