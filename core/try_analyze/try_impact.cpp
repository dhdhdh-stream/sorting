#include "try_impact.h"

#include "constants.h"
#include "try_instance.h"
#include "try_scope_step.h"

using namespace std;

TryImpact::TryImpact() {
	this->overall_count = 0;
	this->overall_impact = 0.0;
}

void TryImpact::calc_impact(TryInstance* try_instance,
							int index,
							double& sum_impacts) {
	sum_impacts += this->overall_impact;

	{
		map<pair<vector<int>,vector<int>>, pair<int,double>>::iterator it = this->start_impacts.find(try_instance->start);
		if (it != this->start_impacts.end()) {
			sum_impacts += it->second.second;
		}
	}

	for (int s_index = 0; s_index < index; s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_pre_impacts.find(action);
			if (it != this->action_pre_impacts.end()) {
				sum_impacts += it->second.second;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it != this->node_pre_impacts.end()) {
					sum_impacts += it->second.second;
				}
			}
		}
	}
	for (int s_index = index + 1; s_index < (int)try_instance->step_types.size(); s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_post_impacts.find(action);
			if (it != this->action_post_impacts.end()) {
				sum_impacts += it->second.second;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_post_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it != this->node_post_impacts.end()) {
					sum_impacts += it->second.second;
				}
			}
		}
	}

	{
		map<pair<vector<int>,vector<int>>, pair<int,double>>::iterator it = this->exit_impacts.find(try_instance->exit);
		if (it != this->exit_impacts.end()) {
			sum_impacts += it->second.second;
		}
	}
}

void TryImpact::update(double impact_diff,
					   TryInstance* try_instance,
					   int index) {
	this->overall_count++;
	this->overall_impact = (1.0 - 1.0 / this->overall_count) * this->overall_impact
		+ 1.0 / this->overall_count * impact_diff;

	{
		map<pair<vector<int>,vector<int>>, pair<int,double>>::iterator it = this->start_impacts.find(try_instance->start);
		if (it == this->start_impacts.end()) {
			it = this->start_impacts.insert({try_instance->start, {0, 0.0}}).first;
		}
		it->second.first++;
		it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
			+ 1.0 / it->second.first * impact_diff;
	}

	for (int s_index = 0; s_index < index; s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_pre_impacts.find(action);
			if (it == this->action_pre_impacts.end()) {
				it = this->action_pre_impacts.insert({action, {0, 0.0}}).first;
			}
			it->second.first++;
			it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
				+ 1.0 / it->second.first * impact_diff;
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it == this->node_pre_impacts.end()) {
					it = this->node_pre_impacts.insert({try_scope_step->original_nodes[n_index], {0, 0.0}}).first;
				}
				it->second.first++;
				it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
					+ 1.0 / it->second.first * impact_diff;
			}
		}
	}
	for (int s_index = index + 1; s_index < (int)try_instance->step_types.size(); s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_post_impacts.find(action);
			if (it == this->action_post_impacts.end()) {
				it = this->action_post_impacts.insert({action, {0, 0.0}}).first;
			}
			it->second.first++;
			it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
				+ 1.0 / it->second.first * impact_diff;
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_post_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it == this->node_post_impacts.end()) {
					it = this->node_post_impacts.insert({try_scope_step->original_nodes[n_index], {0, 0.0}}).first;
				}
				it->second.first++;
				it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
					+ 1.0 / it->second.first * impact_diff;
			}
		}
	}

	{
		map<pair<vector<int>,vector<int>>, pair<int,double>>::iterator it = this->exit_impacts.find(try_instance->exit);
		if (it == this->exit_impacts.end()) {
			it = this->exit_impacts.insert({try_instance->exit, {0, 0.0}}).first;
		}
		it->second.first++;
		it->second.second = (1.0 - 1.0 / it->second.first) * it->second.second
			+ 1.0 / it->second.first * impact_diff;
	}
}
