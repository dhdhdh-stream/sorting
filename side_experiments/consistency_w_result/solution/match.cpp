#include "match.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Match::Match() {
	// do nothing
}

Match::Match(ifstream& input_file,
			 Solution* parent_solution) {
	string num_layers_line;
	getline(input_file, num_layers_line);
	int num_layers = stoi(num_layers_line);
	for (int l_index = 0; l_index < num_layers; l_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->scope_context.push_back(parent_solution->scopes[stoi(scope_id_line)]);

		string node_id_line;
		getline(input_file, node_id_line);
		this->node_context.push_back(stoi(node_id_line));
	}

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);

	string constant_line;
	getline(input_file, constant_line);
	this->constant = stod(constant_line);

	string standard_deviation_line;
	getline(input_file, standard_deviation_line);
	this->standard_deviation = stod(standard_deviation_line);

	this->is_init = true;
}

void Match::eval(vector<double>& obs,
				 RunHelper& run_helper,
				 ScopeHistory* scope_history) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
		.find(this->node_context[0]);
	if (it != scope_history->node_histories.end()) {
		ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

		double predicted_score = obs[0] * this->weight + this->constant;
		if (abs(early_history->obs_history[0] - predicted_score) < MIN_STANDARD_DEVIATION) {
			run_helper.match_factors.push_back(true);
		} else {
			run_helper.match_factors.push_back(false);
		}
	}
}

bool Match::should_delete(Scope* scope,
						  int node_id) {
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		if (this->scope_context[l_index] == scope
				&& this->node_context[l_index] == node_id) {
			return true;
		}
	}

	return false;
}

bool Match::should_delete(Scope* scope) {
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		if (this->scope_context[l_index] == scope) {
			return true;
		}
	}

	return false;
}

void Match::replace_obs_node(Scope* scope,
							 int original_node_id,
							 int new_node_id) {
	if (this->scope_context.back() == scope
			&& this->node_context.back() == original_node_id) {
		this->node_context.back() = new_node_id;
	}
}

void Match::clean() {
	this->datapoints.clear();
}

void Match::update(bool& is_still_needed) {
	ObsNode* early_obs_node = (ObsNode*)this->scope_context[0]->nodes[this->node_context[0]];
	if (early_obs_node->is_fixed_point && this->parent->is_fixed_point) {
		is_still_needed = false;
		return;
	}

	double early_sum_vals = 0.0;
	double later_sum_vals = 0.0;
	for (int d_index = 0; d_index < (int)this->datapoints.size(); d_index++) {
		early_sum_vals += this->datapoints[d_index].first;
		later_sum_vals += this->datapoints[d_index].second;
	}
	double early_average_val = early_sum_vals / (int)this->datapoints.size();
	double later_average_val = later_sum_vals / (int)this->datapoints.size();

	double early_sum_variances = 0.0;
	double later_sum_variances = 0.0;
	for (int d_index = 0; d_index < (int)this->datapoints.size(); d_index++) {
		early_sum_variances += (this->datapoints[d_index].first - early_average_val)
			* (this->datapoints[d_index].first - early_average_val);
		later_sum_variances += (this->datapoints[d_index].second - later_average_val)
			* (this->datapoints[d_index].second - later_average_val);
	}
	double early_standard_deviation = sqrt(early_sum_variances / (int)this->datapoints.size());
	double later_standard_deviation = sqrt(later_sum_variances / (int)this->datapoints.size());

	if (early_standard_deviation < MIN_STANDARD_DEVIATION
			&& later_standard_deviation < MIN_STANDARD_DEVIATION) {
		is_still_needed = true;
	} else if (early_standard_deviation < MIN_STANDARD_DEVIATION) {
		is_still_needed = false;
	} else if (later_standard_deviation < MIN_STANDARD_DEVIATION) {
		is_still_needed = false;
	} else {
		double sum_covariance = 0.0;
		for (int d_index = 0; d_index < (int)this->datapoints.size(); d_index++) {
			sum_covariance += (this->datapoints[d_index].first - early_average_val)
				* (this->datapoints[d_index].second - later_average_val);
		}
		double covariance = sum_covariance / (int)this->datapoints.size();

		double pcc = covariance / early_standard_deviation
			/ later_standard_deviation;
		if (abs(pcc) < MATCH_MIN_PCC) {
			is_still_needed = false;
		} else {
			is_still_needed = true;
		}
	}

	if (is_still_needed) {
		Eigen::MatrixXd inputs(this->datapoints.size(), 2);
		Eigen::VectorXd outputs(this->datapoints.size());
		for (int d_index = 0; d_index < (int)this->datapoints.size(); d_index++) {
			inputs(d_index, 0) = 1.0;
			inputs(d_index, 1) = this->datapoints[d_index].second;
			outputs(d_index) = this->datapoints[d_index].first;
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			is_still_needed = false;
			return;
		}

		this->constant = weights(0);
		this->weight = weights(1);

		Eigen::VectorXd predicted = inputs * weights;
		Eigen::VectorXd diff = outputs - predicted;

		double sum_variance = 0.0;
		for (int d_index = 0; d_index < (int)this->datapoints.size(); d_index++) {
			sum_variance += diff(d_index) * diff(d_index);
		}
		this->standard_deviation = sqrt(sum_variance / (int)this->datapoints.size());
		if (this->standard_deviation < MIN_STANDARD_DEVIATION) {
			this->standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->is_init = true;

		is_still_needed = true;
	}
}

void Match::save(ofstream& output_file) {
	output_file << this->scope_context.size() << endl;
	for (int l_index = 0; l_index < (int)this->scope_context.size(); l_index++) {
		output_file << this->scope_context[l_index]->id << endl;
		output_file << this->node_context[l_index] << endl;
	}

	output_file << this->weight << endl;
	output_file << this->constant << endl;
	output_file << this->standard_deviation << endl;
}
