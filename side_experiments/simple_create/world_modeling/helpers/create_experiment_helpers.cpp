#include "helpers.h"

#include "connection_experiment.h"
#include "experiment.h"
#include "globals.h"
#include "hidden_state.h"
#include "hmm.h"

using namespace std;

void create_commit(vector<HiddenState*>& state_history,
				   vector<int>& action_history,
				   int h_index,
				   int experiment_length,
				   vector<HiddenState*>& experiment_states) {
	for (int l_index = 0; l_index < experiment_length; l_index++) {
		experiment_states.push_back(new HiddenState());
	}

	for (int l_index = 0; l_index < experiment_length-1; l_index++) {
		experiment_states[l_index]->transitions[action_history[h_index + 1 + l_index]] = experiment_states[l_index+1];
	}
}

void create_cancel(vector<HiddenState*>& state_history,
				   vector<int>& action_history,
				   int h_index,
				   int experiment_length,
				   vector<HiddenState*>& experiment_states) {
	for (int l_index = 0; l_index < experiment_length; l_index++) {
		experiment_states.push_back(new HiddenState());
	}

	for (int l_index = 0; l_index < experiment_length-1; l_index++) {
		for (map<int, HiddenState*>::iterator it = state_history[h_index + 1 + l_index]->transitions.begin();
				it != state_history[h_index + 1 + l_index]->transitions.end(); it++) {
			if (it->first != action_history[h_index + 1 + l_index]) {
				experiment_states[l_index]->transitions[it->first] = it->second;
			}
		}
		experiment_states[l_index]->transitions[action_history[h_index + 1 + l_index]] = experiment_states[l_index+1];
	}
}

void create_experiment(vector<HiddenState*>& state_history,
					   vector<int>& action_history) {
	uniform_int_distribution<int> history_distribution(0, state_history.size()-1);
	int h_index = history_distribution(generator);

	uniform_int_distribution<int> type_distribution(0, 1);
	if (type_distribution(generator) == 0) {
		geometric_distribution<int> length_distribution(0.5);
		int experiment_length = 1 + length_distribution(generator);
		if (h_index + experiment_length > (int)state_history.size()) {
			experiment_length = (int)state_history.size() - h_index;
		}

		vector<HiddenState*> experiment_states;

		uniform_int_distribution<int> experiment_type_distribution(0, 1);
		if (experiment_type_distribution(generator) == 0) {
			create_commit(state_history,
						  action_history,
						  h_index,
						  experiment_length,
						  experiment_states);
		} else {
			create_cancel(state_history,
						  action_history,
						  h_index,
						  experiment_length,
						  experiment_states);
		}

		Experiment* new_experiment = new Experiment(state_history[h_index],
													experiment_states);

		state_history[h_index]->experiments[action_history[h_index]] = new_experiment;
	} else {
		uniform_int_distribution<int> state_distribution(0, hmm->hidden_states.size()-1);
		ConnectionExperiment* new_connection_experiment = new ConnectionExperiment(
			state_history[h_index],
			hmm->hidden_states[state_distribution(generator)]);

		state_history[h_index]->experiments[action_history[h_index]] = new_connection_experiment;
	}
}
