#include "try_impact.h"

using namespace std;



double TryImpact::calc_impact(TryInstance* try_instance,
							  int index) {
	if (try_instance->step_types[index] == STEP_TYPE_ACTION) {
		int num_impacts = 1;
		double sum_impacts = this->overall_impact;

		for (int s_index = 0; s_index < index; s_index++) {
			if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
				int action = try_instance->actions[s_index];
				map<int, pair<int, double>>::iterator it = this->action_pre_impacts.find(action);
				if (it != this->action_pre_impacts.end()) {
					num_impacts++;
					sum_impacts += it->second.second;
				}
			} else {
				TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
				int try_scope_step_num_impacts = 0;
				double try_scope_step_sum_impacts = 0.0;
				for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
					map<AbstractNode*, pair<int, double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
					if (it != this->node_pre_impacts.end()) {
						try_scope_step_num_impacts++;
						try_scope_step_sum_impacts += it->second.second;
					}
				}
				if (try_scope_step_num_impacts > 0) {
					num_impacts++;
					sum_impacts += try_scope_step_sum_impacts/try_scope_step_num_impacts;
				}
			}
		}
		for (int s_index = index + 1; s_index < (int)try_instance->step_types.size(); s_index++) {
			if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {

			} else {

			}
		}

		return sum_impacts / num_impacts;
	} else {

	}
}
