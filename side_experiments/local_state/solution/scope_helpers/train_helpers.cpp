#include "scope.h"

#include "abstract_node.h"
#include "globals.h"
#include "init_network.h"
#include "score_network.h"
#include "obs_network.h"

using namespace std;

void Scope::train_activate(ScopeHistory* history,
						   bool allow_drop,
						   Eigen::VectorXf& state,
						   TrainScopeHistory* train_scope_history) {
	if (allow_drop) {
		uniform_int_distribution<int> drop_distribution(0, 19);
		train_scope_history->is_drop = drop_distribution(generator) == 0;
	} else {
		train_scope_history->is_drop = false;
	}

	if (!train_scope_history->is_drop) {
		this->start_obs_network->activate(state,
										  history->obs);
		train_scope_history->start_obs_network_history = new ObsNetworkHistory(this->start_obs_network);
		this->start_obs_network->save(train_scope_history->start_obs_network_history);

		train_scope_history->start_init_network_histories = vector<InitNetworkHistory*>(this->start_init_networks.size(), NULL);
		for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
			if (history->init_is_match[n_index]) {
				this->start_init_networks[n_index]->activate(state,
															 history->obs);
				train_scope_history->start_init_network_histories[n_index] = new InitNetworkHistory(this->start_init_networks[n_index]);
				this->start_init_networks[n_index]->save(train_scope_history->start_init_network_histories[n_index]);
			}
		}
	}

	for (int h_index = 0; h_index < (int)history->node_histories.size(); h_index++) {
		AbstractNode* node = history->node_histories[h_index]->node;
		node->train_step(history->node_histories[h_index],
						 allow_drop,
						 state,
						 train_scope_history);
	}

	// this->end_score_network->activate(state);
	// train_scope_history->end_score_network_history = new ScoreNetworkHistory(this->end_score_network);
	// this->end_score_network->save(train_scope_history->end_score_network_history);
}
