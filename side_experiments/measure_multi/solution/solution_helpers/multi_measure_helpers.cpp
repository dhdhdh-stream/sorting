#include "solution_helpers.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

using namespace std;

void multi_measure_calc(vector<vector<double>>& influences,
						vector<double>& target_vals,
						vector<double>& multi_scores) {
	int num_experiments = (int)influences[0].size();
	int num_instances = (int)influences.size();

	double sum_target_vals = 0.0;
	for (int h_index = 0; h_index < num_instances; h_index++) {
		sum_target_vals += target_vals[h_index];
	}
	double average_target_val = sum_target_vals / num_instances;

	Eigen::MatrixXd inputs(num_instances, num_experiments + 1);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		for (int e_index = 0; e_index < num_experiments; e_index++) {
			inputs(h_index, e_index) = influences[h_index][e_index];
		}
		inputs(h_index, num_experiments) = 1.0;
	}
	Eigen::VectorXd outputs(num_instances);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		outputs(h_index) = target_vals[h_index] - average_target_val;
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		weights = Eigen::VectorXd(num_experiments + 1);
		for (int e_index = 0; e_index < num_experiments + 1; e_index++) {
			weights(e_index) = 0.0;
		}
	}

	multi_scores = vector<double>(num_experiments);
	for (int e_index = 0; e_index < num_experiments; e_index++) {
		multi_scores[e_index] = weights(e_index);
	}
}
