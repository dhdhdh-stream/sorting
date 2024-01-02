#include "try_exit_impact.h"

#include "constants.h"
#include "try_instance.h"
#include "try_scope_step.h"

using namespace std;

TryExitImpact::TryExitImpact() {
	this->overall_count = 0;
	this->overall_impact = 0.0;
}

TryExitImpact::TryExitImpact(ifstream& input_file) {
	string overall_count_line;
	getline(input_file, overall_count_line);
	this->overall_count = stoi(overall_count_line);

	string overall_impact_line;
	getline(input_file, overall_impact_line);
	this->overall_impact = stod(overall_impact_line);

	string action_pre_impacts_size_line;
	getline(input_file, action_pre_impacts_size_line);
	int action_pre_impacts_size = stoi(action_pre_impacts_size_line);
	for (int i_index = 0; i_index < action_pre_impacts_size; i_index++) {
		string action_line;
		getline(input_file, action_line);
		int action = stoi(action_line);

		string count_line;
		getline(input_file, count_line);
		int count = stoi(count_line);

		string impact_line;
		getline(input_file, impact_line);
		double impact = stod(impact_line);

		this->action_pre_impacts[action] = {count, impact};
	}

	string node_pre_impacts_size_line;
	getline(input_file, node_pre_impacts_size_line);
	int node_pre_impacts_size = stoi(node_pre_impacts_size_line);
	for (int i_index = 0; i_index < node_pre_impacts_size; i_index++) {
		string parent_id_line;
		getline(input_file, parent_id_line);
		int parent_id = stoi(parent_id_line);

		string node_id_line;
		getline(input_file, node_id_line);
		int node_id = stoi(node_id_line);

		string count_line;
		getline(input_file, count_line);
		int count = stoi(count_line);

		string impact_line;
		getline(input_file, impact_line);
		double impact = stod(impact_line);

		this->node_pre_impacts[{parent_id, node_id}] = {count, impact};
	}
}

void TryExitImpact::calc_impact(TryInstance* try_instance,
								double& sum_impacts) {
	sum_impacts += this->overall_impact;

	for (int s_index = 0; s_index < (int)try_instance->step_types.size(); s_index++) {
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
}

void TryExitImpact::calc_impact(TryInstance* try_instance,
								int& num_impacts,
								double& sum_impacts) {
	num_impacts++;
	sum_impacts += this->overall_impact;

	for (int s_index = 0; s_index < (int)try_instance->step_types.size(); s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_pre_impacts.find(action);
			if (it == this->action_pre_impacts.end()) {
				it = this->action_pre_impacts.insert({action, {0, 0.0}}).first;
			}
			num_impacts++;
			sum_impacts += it->second.second;
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
				if (it == this->node_pre_impacts.end()) {
					it = this->node_pre_impacts.insert({try_scope_step->original_nodes[n_index], {0, 0.0}}).first;
				}
				num_impacts++;
				sum_impacts += it->second.second;
			}
		}
	}
}

void TryExitImpact::backprop(double impact_diff,
							 TryInstance* try_instance) {
	this->overall_count++;
	if (this->overall_count >= 100) {
		this->overall_impact += 0.01 * impact_diff;
	} else {
		this->overall_impact += 1.0 / this->overall_count * impact_diff;
	}

	for (int s_index = 0; s_index < (int)try_instance->step_types.size(); s_index++) {
		if (try_instance->step_types[s_index] == STEP_TYPE_ACTION) {
			int action = try_instance->actions[s_index];
			map<int, pair<int,double>>::iterator it = this->action_pre_impacts.find(action);
			// it != this->action_pre_impacts.end()
			it->second.first++;
			if (it->second.first >= 100) {
				it->second.second += 0.01 * impact_diff;
			} else {
				it->second.second += 1.0 / it->second.first * impact_diff;
			}
		} else {
			TryScopeStep* try_scope_step = try_instance->potential_scopes[s_index];
			for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
				map<pair<int,int>, pair<int,double>>::iterator it = this->node_pre_impacts.find(try_scope_step->original_nodes[n_index]);
				// it != this->node_pre_impacts.end()
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

void TryExitImpact::save(ofstream& output_file) {
	output_file << this->overall_count << endl;
	output_file << this->overall_impact << endl;

	output_file << this->action_pre_impacts.size() << endl;
	for (map<int, pair<int,double>>::iterator it = this->action_pre_impacts.begin();
			it != this->action_pre_impacts.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second.first << endl;
		output_file << it->second.second << endl;
	}

	output_file << this->node_pre_impacts.size() << endl;
	for (map<pair<int,int>, pair<int,double>>::iterator it = this->node_pre_impacts.begin();
			it != this->node_pre_impacts.end(); it++) {
		output_file << it->first.first << endl;
		output_file << it->first.second << endl;
		output_file << it->second.first << endl;
		output_file << it->second.second << endl;
	}
}
