#include "scope_node.h"

#include "globals.h"
#include "pass_through_network.h"
#include "scope.h"
#include "score_network.h"

using namespace std;

void ScopeNode::train_step(AbstractNodeHistory* history,
						   bool allow_drop,
						   Eigen::VectorXf& state,
						   TrainScopeHistory* train_scope_history) {
	ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history;

	TrainScopeNodeHistory* train_history = new TrainScopeNodeHistory(this);
	train_scope_history->node_histories.push_back(train_history);

	TrainScopeHistory* inner_train_scope_history = new TrainScopeHistory(this->scope);
	train_history->scope_history = inner_train_scope_history;

	Eigen::VectorXf inner_state;
	inner_state.resize(this->scope->num_states);
	inner_state.setConstant(0.0);

	for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->in_pass_through_networks[n_index];
		double val = state(pass_through_network->front_state_index);
		inner_state(pass_through_network->back_state_index) += val;
	}

	this->scope->train_activate(scope_node_history->scope_history,
								allow_drop,
								inner_state,
								inner_train_scope_history);

	for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->out_pass_through_networks[n_index];
		double val = inner_state(pass_through_network->front_state_index);
		state(pass_through_network->back_state_index) += val;
	}
}
