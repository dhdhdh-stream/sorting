#include "scope_node.h"

#include "globals.h"
#include "pass_through_network.h"
#include "predict_network.h"
#include "scope.h"
#include "score_network.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::train_step(AbstractNodeHistory* history,
						   bool allow_drop,
						   Eigen::VectorXf& state,
						   int run_type,
						   TrainScopeHistory* train_scope_history) {
	ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history;

	TrainScopeNodeHistory* train_history = new TrainScopeNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	TrainScopeHistory* inner_train_scope_history = new TrainScopeHistory(this->scope);
	train_history->scope_history = inner_train_scope_history;

	// Eigen::VectorXf starting_state = state;

	Eigen::VectorXf inner_state;
	inner_state.resize(this->scope->num_states);
	inner_state.setConstant(0.0);

	if (allow_drop) {
		uniform_int_distribution<int> drop_distribution(0, 49);
		train_history->in_is_drop = drop_distribution(generator) == 0;
	} else {
		train_history->in_is_drop = false;
	}

	if (!train_history->in_is_drop) {
		for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
			PassThroughNetwork* pass_through_network = this->in_pass_through_networks[n_index];
			double val = state(pass_through_network->front_state_index);
			inner_state(pass_through_network->back_state_index) += val;
		}

		this->in_network->activate(state,
								   inner_state);
		train_history->in_network_history = new TransitionNetworkHistory();
		this->in_network->save(train_history->in_network_history);
	}

	this->scope->train_activate(scope_node_history->scope_history,
								allow_drop,
								inner_state,
								run_type,
								inner_train_scope_history);

	if (allow_drop) {
		uniform_int_distribution<int> drop_distribution(0, 49);
		train_history->out_is_drop = drop_distribution(generator) == 0;
	} else {
		train_history->out_is_drop = false;
	}

	if (!train_history->out_is_drop) {
		for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
			PassThroughNetwork* pass_through_network = this->out_pass_through_networks[n_index];
			double val = inner_state(pass_through_network->front_state_index);
			state(pass_through_network->back_state_index) += val;
		}

		this->out_network->activate(inner_state,
									state);
		train_history->out_network_history = new TransitionNetworkHistory();
		this->out_network->save(train_history->out_network_history);
	}

	// Eigen::VectorXf state_diff = state - starting_state;

	// this->predict_network->backprop(starting_state,
	// 								state_diff);
}
