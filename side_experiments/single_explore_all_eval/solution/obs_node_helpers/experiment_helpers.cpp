#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "damage.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			this,
			false,
			wrapper);
	} else {
		if (this->parent->id != -1 && wrapper->is_damage) {
			uniform_int_distribution<int> damage_distribution(0, 49);
			if (damage_distribution(generator) == 0) {
				history->damage = new Damage(this,
											 false,
											 wrapper);
				history->damage->experiment_check_activate(
					this,
					false,
					wrapper);
			}
		}
	}
}
