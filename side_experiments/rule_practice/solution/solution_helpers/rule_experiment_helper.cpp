#include "solution_helpers.h"

using namespace std;

const int MEASURE_ITERS = 10000;

void insert_val(double new_val,
				vector<double>& best_vals) {
	bool should_insert;
	if (best_vals.size() < MEASURE_ITERS * 0.05) {
		should_insert = true;
	} else {
		if (new_val > best_vals.back()) {
			best_vals.pop_back();
			should_insert = true;
		} else {
			should_insert = false;
		}
	}

	if (should_insert) {
		int insert_index = 0;
		for (int v_index = (int)best_vals.size()-1; v_index >= 0; v_index--) {
			if (new_val <= best_vals[v_index]) {
				insert_index = v_index+1;
				break;
			}
		}
		best_vals.insert(best_vals.begin() + insert_index, new_val);
	}
}

void rule_experiment(vector<Rule*>& rules,
					 double& new_average_val,
					 double& new_top_5_percentile,
					 double& new_top_5_percentile_average_val) {
	double sum_vals = 0.0;
	vector<double> best_vals;
	for (int i_index = 0; i_index < MEASURE_ITERS; i_index++) {
		bool is_fail;
		double result;
		run(rules,
			is_fail,
			result);

		if (is_fail) {
			return;
		}

		sum_vals += result;
		insert_val(result,
				   best_vals);
	}

	new_average_val = sum_vals / MEASURE_ITERS;
	new_top_5_percentile = best_vals.back();
	double sum_top_5_vals = 0.0;
	for (int v_index = 0; v_index < (int)best_vals.size(); v_index++) {
		sum_top_5_vals += best_vals[v_index];
	}
	new_top_5_percentile_average_val = sum_top_5_vals / (int)best_vals.size();
}

void rule_experiment(vector<Rule*>& rules,
					 Rule* potential_rule,
					 double& new_average_val,
					 double& new_top_5_percentile,
					 double& new_top_5_percentile_average_val,
					 double& potential_percentage) {
	double sum_vals = 0.0;
	vector<double> best_vals;
	int num_hit_potential = 0;
	for (int i_index = 0; i_index < MEASURE_ITERS; i_index++) {
		bool is_fail;
		double result;
		bool hit_potential;
		run(rules,
			potential_rule,
			is_fail,
			result,
			hit_potential);

		if (is_fail) {
			return;
		}

		sum_vals += result;
		insert_val(result,
				   best_vals);
		if (hit_potential) {
			num_hit_potential++;
		}
	}

	new_average_val = sum_vals / MEASURE_ITERS;
	new_top_5_percentile = best_vals.back();
	double sum_top_5_vals = 0.0;
	for (int v_index = 0; v_index < (int)best_vals.size(); v_index++) {
		sum_top_5_vals += best_vals[v_index];
	}
	new_top_5_percentile_average_val = sum_top_5_vals / (int)best_vals.size();
	potential_percentage = (double)num_hit_potential / (double)MEASURE_ITERS;
}
