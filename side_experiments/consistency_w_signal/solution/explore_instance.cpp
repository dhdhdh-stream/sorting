#include "explore_instance.h"

#include <iostream>
#include <sstream>

#include "abstract_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "network.h"
#include "scope.h"

using namespace std;

ExploreInstance::ExploreInstance() {
	this->new_scope = NULL;
}

ExploreInstance::~ExploreInstance() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int h_index = 0; h_index < (int)this->post_scope_histories.size(); h_index++) {
		delete this->post_scope_histories[h_index];
	}
}

void ExploreInstance::calc_consistency() {
	double sum_consistency = 0.0;
	int count = 0;
	for (int h_index = 0; h_index < (int)this->post_scope_histories.size(); h_index++) {
		Scope* scope = this->post_scope_histories[h_index]->scope;
		if (scope->consistency_network != NULL) {
			vector<double> input = this->post_scope_histories[h_index]->pre_obs;
			input.insert(input.end(), this->post_scope_histories[h_index]->post_obs.begin(),
				this->post_scope_histories[h_index]->post_obs.end());

			scope->consistency_network->activate(input);
			double consistency = scope->consistency_network->output->acti_vals[0];
			/**
			 * - allow to go below -1.0 to help distinguish between bad and very bad
			 *   - sigmoid not better (?)
			 */
			if (consistency >= 3.0) {
				sum_consistency += 3.0;
				count++;
			} else if (consistency <= -3.0) {
				sum_consistency += -3.0;
				count++;
			} else {
				sum_consistency += consistency;
				count++;
			}
		}
	}

	if (count == 0) {
		this->consistency = 0.0;
	} else {
		this->consistency = sum_consistency / (double)count;
	}
}

void ExploreInstance::print() {
	stringstream ss;
	ss << "this->experiment->scope_context->id: " << this->experiment->scope_context->id << "; ";
	ss << "this->experiment->node_context->id: " << this->experiment->node_context->id << "; ";
	ss << "this->experiment->is_branch: " << this->experiment->is_branch << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << this->actions[s_index];
		} else {
			ss << " E" << this->scopes[s_index]->id;
		}
	}
	ss << "; ";

	if (this->exit_next_node == NULL) {
		ss << "this->exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->exit_next_node->id: " << this->exit_next_node->id << "; ";
	}

	cout << ss.str() << endl;
}
