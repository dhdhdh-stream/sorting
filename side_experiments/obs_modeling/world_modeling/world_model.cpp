#include "world_model.h"

using namespace std;



bool WorldModel::activate(vector<double>& obs_sequence,
						  vector<Action*>& action_sequence,
						  vector<vector<int>>& action_state_sequence,
						  vector<vector<double>>& state_vals_sequence;
						  double target_val) {
	RunHelper run_helper;

	vector<WorldState*> state_history;
	vector<int> sequence_index_history;

	int starting_sequence_length = (int)obs_sequence.size();

	WorldState* curr_state = this->world_states[0];
	int curr_sequence_index = 0;
	while (obs_sequence.size() > 0) {
		state_history.push_back(curr_state);
		sequence_index_history.push_back(starting_sequence_length - (int)obs_sequence.size());
		curr_state->activate(curr_state,
							 curr_sequence_index,
							 obs_sequence,
							 action_sequence,
							 action_state_sequence,
							 state_vals_sequence,
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
												 state_vals);

		if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			for (int h_index = 0; h_index < (int)this->hidden_states.size(); h_index++) {
				this->hidden_states[h_index]->success_reset();
			}

			ofstream output_file;
			output_file.open("display.txt");
			save_for_display(output_file);
			output_file.close();

			return true;
		} else if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_FAIL) {
			map<int, AbstractExperiment*>::iterator it = run_helper.selected_experiment->parent->experiments.begin();
			while (true) {
				if (it->second == run_helper.selected_experiment) {
					break;
				}
				it++;
			}
			run_helper.selected_experiment->parent->experiments.erase(it);
			delete run_helper.selected_experiment;
		}
	} else {
		if (run_helper.experiments_seen.size() == 0) {
			create_experiment(state_history,
							  action_history,
							  action_state_history,
							  sequence_index_history);
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

void WorldModel::measure_activate() {

}
