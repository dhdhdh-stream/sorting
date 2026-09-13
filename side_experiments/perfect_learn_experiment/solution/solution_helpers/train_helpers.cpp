#include "solution_helpers.h"

#include "action_network.h"
#include "constants.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

void train_helper(vector<double>& obs,
				  vector<int>& actions,
				  double target_val,
				  Solution* solution) {
	Eigen::VectorXf state;
	state.resize(NUM_STATES);
	state.setConstant(0.0);

	solution->obs_network->activate(state,
									obs);

	vector<ActionNetworkHistory*> action_network_histories;
	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		solution->action_networks[actions[a_index]]->activate(state);
		ActionNetworkHistory* action_network_history = new ActionNetworkHistory();
		solution->action_networks[actions[a_index]]->save(action_network_history);

		action_network_histories.push_back(action_network_history);
	}

	solution->score_network->activate(state);

	Eigen::VectorXf state_error;
	state_error.resize(NUM_STATES);
	state_error.setConstant(0.0);

	solution->score_network->backprop(target_val,
									  state_error);

	for (int a_index = (int)actions.size()-1; a_index >= 0; a_index--) {
		solution->action_networks[actions[a_index]]->load(action_network_histories[a_index]);
		solution->action_networks[actions[a_index]]->backprop(state_error);

		delete action_network_histories[a_index];
	}

	solution->obs_network->backprop(state_error);

	vector<bool> hit_actions(solution->action_networks.size(), false);
	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		hit_actions[actions[a_index]] = true;
	}

	solution->obs_network->update();
	for (int a_index = 0; a_index < (int)solution->action_networks.size(); a_index++) {
		if (hit_actions[a_index]) {
			solution->action_networks[a_index]->update();
		}
	}
	solution->score_network->update();
}
