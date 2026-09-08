#include "scope.h"

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "score_network.h"
#include "obs_network.h"

using namespace std;

void Scope::train_activate(ScopeHistory* history,
						   bool allow_drop,
						   Eigen::VectorXf& state,
						   int run_type,
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
		train_scope_history->start_obs_network_history = new ObsNetworkHistory();
		this->start_obs_network->save(train_scope_history->start_obs_network_history);
	}

	for (int h_index = 0; h_index < (int)history->node_histories.size(); h_index++) {
		AbstractNode* node = history->node_histories[h_index]->node;
		node->train_step(history->node_histories[h_index],
						 allow_drop,
						 state,
						 run_type,
						 train_scope_history);
	}

	if (run_type == RUN_TYPE_EXPLORE) {
		this->end_score_network->activate(state);
		train_scope_history->end_score_network_history = new ScoreNetworkHistory();
		this->end_score_network->save(train_scope_history->end_score_network_history);
	}
}
