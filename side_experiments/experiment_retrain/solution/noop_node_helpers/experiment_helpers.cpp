#include "noop_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void NoopNode::experiment_step(vector<double>& obs,
							   int& action,
							   bool& is_next,
							   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	NoopNodeHistory* history = new NoopNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
			InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
			this->init_networks[n_index]->save(init_network_history);
			wrapper->network_histories.push_back(init_network_history);

			if (!wrapper->should_explore) {
				this->prev_init_networks[n_index]->activate(wrapper->prev_state,
															obs);
			}
		}
	}

	if (this->dependencies.size() > 0) {
		history->state = wrapper->state;
		history->obs = obs;
	}

	if (!wrapper->should_explore) {
		this->score_network->activate(wrapper->state);
		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
		this->score_network->save(score_network_history);
		wrapper->network_histories.push_back(score_network_history);
	} else {
		this->explore_score_network->activate(wrapper->state);
		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->explore_score_network);
		this->explore_score_network->save(score_network_history);
		wrapper->network_histories.push_back(score_network_history);
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			obs,
			wrapper);
	}
}
