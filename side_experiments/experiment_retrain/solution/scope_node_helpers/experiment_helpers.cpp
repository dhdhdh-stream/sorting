#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);

	this->scope->experiment_start_activate(obs,
										   wrapper);
}

void ScopeNode::experiment_exit_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->scope_history = wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	if (!wrapper->should_explore) {
		this->score_network->activate(wrapper->state);
		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
		this->score_network->save(score_network_history);
		wrapper->network_histories.push_back(score_network_history);
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			obs,
			wrapper);
	}
}
