#include "action_node.h"

#include "action_network.h"
#include "globals.h"
#include "init_network.h"
#include "obs_network.h"
#include "scope.h"
#include "score_network.h"

using namespace std;

void ActionNode::train_step(AbstractNodeHistory* history,
							bool allow_drop,
							Eigen::VectorXf& state,
							TrainScopeHistory* train_scope_history) {
	ActionNodeHistory* action_node_history = (ActionNodeHistory*)history;

	bool is_drop;
	if (allow_drop && !this->is_generic) {
		uniform_int_distribution<int> drop_distribution(0, 19);
		is_drop = drop_distribution(generator) == 0;
	} else {
		is_drop = false;
	}

	if (!is_drop) {
		TrainActionNodeHistory* train_history = new TrainActionNodeHistory(this);
		train_scope_history->node_histories.push_back(train_history);

		this->action_network->activate(state);
		train_history->action_network_history = new ActionNetworkHistory(this->action_network);
		this->action_network->save(train_history->action_network_history);

		this->obs_network->activate(state,
									action_node_history->obs);
		train_history->obs_network_history = new ObsNetworkHistory(this->obs_network);
		this->obs_network->save(train_history->obs_network_history);

		train_history->init_network_histories = vector<InitNetworkHistory*>(this->init_networks.size(), NULL);
		for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
			if (action_node_history->init_is_match[n_index]) {
				this->init_networks[n_index]->activate(state,
													   action_node_history->obs);
				train_history->init_network_histories[n_index] = new InitNetworkHistory(this->init_networks[n_index]);
				this->init_networks[n_index]->save(train_history->init_network_histories[n_index]);
			}
		}

		// if (!this->is_generic) {
		// 	this->score_network->activate(state);
		// 	train_history->score_network_history = new ScoreNetworkHistory(this->score_network);
		// 	this->score_network->save(train_history->score_network_history);
		// }
	}
}
