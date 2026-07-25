#include "solution_helpers.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void print_run_helper(ScopeHistory* scope_history) {
	vector<AbstractNodeHistory*> in_order(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		in_order[h_it->second->index] = h_it->second;
	}

	for (int h_index = 0; h_index < (int)in_order.size(); h_index++) {
		AbstractNode* node = in_order[h_index]->node;

		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)in_order[h_index];
			print_run_helper(scope_node_history->scope_history);
		}

		cout << "node->parent->id: " << node->parent->id << endl;
		cout << "node->id: " << node->id << endl;
	}
}

void print_state_helper(SolutionWrapper* wrapper) {
	Eigen::VectorXf state;
	state.resize(wrapper->solution->num_states);
	state.setConstant(0.0);
	for (int h_index = 0; h_index < (int)wrapper->partial_network_histories.size(); h_index++) {
		switch (wrapper->partial_network_histories[h_index]->network->type) {
		case NETWORK_TYPE_OBS:
			{
				ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
				ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
				obs_network->load(obs_network_history);

				state += obs_network->output->acti_vals;

				cout << "NETWORK_TYPE_OBS" << endl;
				cout << "obs_network->state_norms:";
				for (int s_index = 0; s_index < (int)obs_network->state_norms.size(); s_index++) {
					cout << " " << obs_network->state_norms(s_index);
				}
				cout << endl;
				cout << "obs_network->state_input->acti_vals:";
				for (int s_index = 0; s_index < (int)obs_network->state_input->acti_vals.size(); s_index++) {
					cout << " " << obs_network->state_input->acti_vals(s_index);
				}
				cout << endl;
				cout << "state:";
				for (int s_index = 0; s_index < (int)state.size(); s_index++) {
					cout << " " << state(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_SCORE:
			{
				ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
				ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
				score_network->load(score_network_history);

				cout << "NETWORK_TYPE_SCORE" << endl;
				cout << "score_network->state_norms:";
				for (int s_index = 0; s_index < (int)score_network->state_norms.size(); s_index++) {
					cout << " " << score_network->state_norms(s_index);
				}
				cout << endl;
				cout << "score_network->state_input->acti_vals:";
				for (int s_index = 0; s_index < (int)score_network->state_input->acti_vals.size(); s_index++) {
					cout << " " << score_network->state_input->acti_vals(s_index);
				}
				cout << endl;
				cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
			}
			break;
		case NETWORK_TYPE_ACTION:
			{
				ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
				ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
				action_network->load(action_network_history);

				state += action_network->output->acti_vals;

				cout << "NETWORK_TYPE_ACTION" << endl;
				cout << "action_network->state_norms:";
				for (int s_index = 0; s_index < (int)action_network->state_norms.size(); s_index++) {
					cout << " " << action_network->state_norms(s_index);
				}
				cout << endl;
				cout << "action_network->state_input->acti_vals:";
				for (int s_index = 0; s_index < (int)action_network->state_input->acti_vals.size(); s_index++) {
					cout << " " << action_network->state_input->acti_vals(s_index);
				}
				cout << endl;
				cout << "state:";
				for (int s_index = 0; s_index < (int)state.size(); s_index++) {
					cout << " " << state(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_INIT:
			{
				InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
				InitNetwork* init_network = (InitNetwork*)init_network_history->network;
				init_network->load(init_network_history);

				for (int i_index = 0; i_index < (int)init_network->init_states.size(); i_index++) {
					state(init_network->init_states[i_index]) += init_network->output->acti_vals(i_index);
				}

				cout << "NETWORK_TYPE_INIT" << endl;
				cout << "init_network->state_norms:";
				for (int s_index = 0; s_index < (int)init_network->state_norms.size(); s_index++) {
					cout << " " << init_network->state_norms(s_index);
				}
				cout << endl;
				cout << "init_network->state_input->acti_vals:";
				for (int s_index = 0; s_index < (int)init_network->state_input->acti_vals.size(); s_index++) {
					cout << " " << init_network->state_input->acti_vals(s_index);
				}
				cout << endl;
				cout << "state:";
				for (int s_index = 0; s_index < (int)state.size(); s_index++) {
					cout << " " << state(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_NEGATE:
			{
				NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->partial_network_histories[h_index];
				NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
				state(negate_network->state_index) = 0.0;

				cout << "NETWORK_TYPE_NEGATE" << endl;
				cout << "negate_network->state_index: " << negate_network->state_index << endl;
			}
			break;
		}
	}
}

void print_error_helper(double target_val,
						SolutionWrapper* wrapper) {
	for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
		switch (wrapper->partial_network_histories[h_index]->network->type) {
		case NETWORK_TYPE_OBS:
			{
				ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
				ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
				obs_network->clear_update_weights();
			}
			break;
		case NETWORK_TYPE_SCORE:
			{
				ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
				ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
				score_network->clear_update_weights();
			}
			break;
		case NETWORK_TYPE_ACTION:
			{
				ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
				ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
				action_network->clear_update_weights();
			}
			break;
		case NETWORK_TYPE_INIT:
			{
				InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
				InitNetwork* init_network = (InitNetwork*)init_network_history->network;
				init_network->clear_update_weights();
			}
			break;
		}
	}

	map<AbstractNetwork*, vector<double>> max_update_history;

	Eigen::VectorXf state_errors;
	state_errors.resize(wrapper->solution->num_states);
	state_errors.setConstant(0.0);
	for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
		AbstractNetwork* network = wrapper->partial_network_histories[h_index]->network;
		map<AbstractNetwork*, vector<double>>::iterator it = max_update_history.find(network);
		if (it == max_update_history.end()) {
			it = max_update_history.insert({network, vector<double>()}).first;
		}

		switch (network->type) {
		case NETWORK_TYPE_OBS:
			{
				ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
				ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
				obs_network->load(obs_network_history);
				obs_network->backprop(state_errors);

				double max_update_size = 0.0;
				obs_network->get_max_update(max_update_size);
				it->second.push_back(max_update_size);

				cout << "NETWORK_TYPE_OBS" << endl;
				cout << "max_update_size: " << max_update_size << endl;
				cout << "obs_network->state_norms:";
				for (int s_index = 0; s_index < (int)obs_network->state_norms.size(); s_index++) {
					cout << " " << obs_network->state_norms(s_index);
				}
				cout << endl;
				cout << "obs_network->state_input->acti_vals:";
				for (int s_index = 0; s_index < (int)obs_network->state_input->acti_vals.size(); s_index++) {
					cout << " " << obs_network->state_input->acti_vals(s_index);
				}
				cout << endl;
				cout << "obs_network->output->acti_vals:";
				for (int s_index = 0; s_index < (int)obs_network->output->acti_vals.size(); s_index++) {
					cout << " " << obs_network->output->acti_vals(s_index);
				}
				cout << endl;
				cout << "state_errors:";
				for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
					cout << " " << state_errors(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_SCORE:
			{
				ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
				ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
				score_network->load(score_network_history);
				
				double diff = target_val - score_network->output->acti_vals(0);

				score_network->backprop(target_val,
										state_errors);

				cout << "NETWORK_TYPE_SCORE" << endl;
				cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
				cout << "diff: " << diff << endl;
				cout << "state_errors:";
				for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
					cout << " " << state_errors(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_ACTION:
			{
				ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
				ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
				action_network->load(action_network_history);
				action_network->backprop(state_errors);

				double max_update_size = 0.0;
				action_network->get_max_update(max_update_size);
				it->second.push_back(max_update_size);

				cout << "NETWORK_TYPE_ACTION" << endl;
				cout << "max_update_size: " << max_update_size << endl;
				cout << "state_errors:";
				for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
					cout << " " << state_errors(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_INIT:
			{
				InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
				InitNetwork* init_network = (InitNetwork*)init_network_history->network;
				init_network->load(init_network_history);
				init_network->backprop(state_errors);

				double max_update_size = 0.0;
				init_network->get_max_update(max_update_size);
				it->second.push_back(max_update_size);

				cout << "NETWORK_TYPE_INIT" << endl;
				cout << "max_update_size: " << max_update_size << endl;
				cout << "state_errors:";
				for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
					cout << " " << state_errors(s_index);
				}
				cout << endl;
			}
			break;
		case NETWORK_TYPE_NEGATE:
			{
				NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->partial_network_histories[h_index];
				NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
				state_errors(negate_network->state_index) = 0.0;
			}
			break;
		}
	}

	for (map<AbstractNetwork*, vector<double>>::iterator it = max_update_history.begin();
			it != max_update_history.end(); it++) {
		cout << "it->second.size(): " << it->second.size() << endl;
		for (int h_index = 0; h_index < (int)it->second.size(); h_index++) {
			cout << it->second[h_index] << " ";
		}
		cout << endl;
	}
}
