#include "try_impact.h"

using namespace std;

TryImpact::TryImpact() {
	this->overall_count = 0;
	this->overall_impact = 0.0;
}

double TryImpact::calc_impact(TryInstance* try_instance,
							  int index) {
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
			int action = try_instance->actions[s_index];
			map<int, pair<int, double>>::iterator it = this->action_post_impacts.find(action);
			if (it != this->action_post_impacts.end()) {
				num_impacts++;
				sum_impacts += it->second.second;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			int try_scope_step_num_impacts = 0;
			double try_scope_step_sum_impacts = 0.0;
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<AbstractNode*, pair<int, double>>::iterator it = this->node_post_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it != this->node_post_impacts.end()) {
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

	return sum_impacts / num_impacts;
}

void TryImpact::backprop(double impact_diff,
						 TryInstance* try_instance,
						 int index) {
	this->overall_count++;
	if (this->overall_count >= 100) {
		this->overall_impact += 0.01 * impact_diff;
	} else {
		this->overall_impact += 1.0 / this->overall_count * impact_diff;
	}

	for (int s_index = 0; s_index < index; s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int, double>>::iterator it = this->action_pre_impacts.find(action);
			if (it == this->action_pre_impacts.end()) {
				it = this->action_pre_impacts.insert({action, {0, 0.0}}).first;
			}
			it->second.first++;
			if (it->second.first >= 100) {
				it->second.second += 0.01 * impact_diff;
			} else {
				it->second.second += 1.0 / it->second.first * impact_diff;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<AbstractNode*, pair<int, double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it == this->node_pre_impacts.end()) {
					it = this->node_pre_impacts.insert({try_scope_step->original_nodes[n_index], {0, 0.0}}).first;
				}
				it->second.first++;
				if (it->second.first >= 100) {
					it->second.second += 0.01 * impact_diff;
				} else {
					it->second.second += 1.0 / it->second.first * impact_diff;
				}
			}
		}
	}
	for (int s_index = index + 1; s_index < (int)try_instance->step_types.size(); s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int, double>>::iterator it = this->action_post_impacts.find(action);
			if (it == this->action_post_impacts.end()) {
				it = this->action_post_impacts.insert({action, {0, 0.0}}).first;
			}
			it->second.first++;
			if (it->second.first >= 100) {
				it->second.second += 0.01 * impact_diff;
			} else {
				it->second.second += 1.0 / it->second.first * impact_diff;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<AbstractNode*, pair<int, double>>::iterator it = this->node_post_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it == this->node_post_impacts.end()) {
					it = this->node_post_impacts.insert({try_scope_step->original_nodes[n_index], {0, 0.0}}).first;
				}
				it->second.first++;
				if (it->second.first >= 100) {
					it->second.second += 0.01 * impact_diff;
				} else {
					it->second.second += 1.0 / it->second.first * impact_diff;
				}
			}
		}
	}
}
