#include "action_node.h"

using namespace std;



void ActionNode::explore_activate(vector<vector<double>>& flat_vals,
								  vector<ForwardContextLayer>& context,
								  RunHelper& run_helper,
								  ActionNodeHistory* history) {
	history->obs_snapshot = flat_vals[0];

	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		int state_index = this->state_network_indexes[n_index];
		if (context.back().states_initialized[state_index]) {
			if (context.back().is_learn_existing[state_index]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->state_networks[n_index]);
				this->state_networks[n_index]->activate(history->obs_snapshot,
														context.back().state_vals->at(state_index),
														network_history);
				history->state_network_indexes.push_back(state_index);
				history->state_network_histories.push_back(network_history);
			} else {
				this->state_networks[n_index]->activate(history->obs_snapshot,
														context.back().state_vals->at(state_index));
			}
		}
	}

	flat_vals.erase(flat_vals.begin());
}

void ActionNode::explore_backprop(vector<BackwardContextLayer>& context,
								  RunHelper& run_helper,
								  ActionNodeHistory* history) {
	for (int n_index = 0; n_index < (int)history->state_network_histories.size(); n_index++) {
		int state_index = history->state_network_indexes[n_index];
		StateNetwork* network = history->state_network_histories[n_index]->network;
		network->backprop_errors_with_no_weight_change(
			context.back().state_errors->at(state_index),
			history->obs_snapshot,
			history->state_network_histories[n_index]);
	}
}

void ActionNode::update_activate(vector<vector<double>>& flat_vals,
								 vector<ForwardContextLayer>& context,
								 RunHelper& run_helper,
								 ActionNodeHistory* history) {
	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		int state_index = this->state_network_indexes[n_index];
		if (context.back().states_initialized[state_index]) {
			this->state_networks[n_index]->activate(flat_vals[0],
													context.back().state_vals->at(state_index));
		}
	}

	history->state_snapshot = *(context.back().state_vals);

	flat_vals.erase(flat_vals.begin());
}

void ActionNode::update_backprop(RunHelper& run_helper,
								 ActionNodeHistory* history) {
	double predicted_score = run_helper.running_average_score;
	for (int s_index = 0; s_index < this->score_scales.size(); s_index++) {
		predicted_score += run_helper.scale_factor*this->score_scales[s_index]*history->state_snapshot[s_index];
	}
	double predicted_score_error = run_helper.target_val - predicted_score;
	predicted_score_error *= run_helper.scale_factor;
	for (int s_index = 0; s_index < this->score_scales.size(); s_index++) {
		this->score_scales[s_index]->backprop(
			predicted_score_error*history->state_snapshot[s_index],
			0.001);
	}
}

void ActionNode::remeasure_activate(vector<vector<double>>& flat_vals,
									vector<ForwardContextLayer>& context,
									RunHelper& run_helper) {
	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		int state_index = this->state_network_indexes[n_index];
		if (context.back().states_initialized[state_index]) {
			this->state_networks[n_index]->activate(flat_vals[0],
													context.back().state_vals->at(state_index));
		}
	}

	flat_vals.erase(flat_vals.begin());
}


