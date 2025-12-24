#include "solution_helpers.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "globals.h"

using namespace std;

bool calc_linear_regression(vector<vector<double>>& inputs,
							vector<double>& outputs,
							vector<double>& weights) {
	Eigen::MatrixXd e_inputs(inputs.size(), inputs[0].size());
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int i_index = 0; i_index < (int)inputs.size(); i_index++) {
		for (int f_index = 0; f_index < (int)inputs[i_index].size(); f_index++) {
			e_inputs(i_index, f_index) = inputs[i_index][f_index]
				+ noise_distribution(generator);
		}
	}

	Eigen::VectorXd e_outputs(outputs.size());
	for (int i_index = 0; i_index < (int)outputs.size(); i_index++) {
		e_outputs(i_index) = outputs[i_index];
	}

	Eigen::VectorXd e_weights;
	try {
		e_weights = e_inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(e_outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		return false;
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	for (int f_index = 0; f_index < (int)inputs[0].size(); f_index++) {
		if (abs(e_weights(f_index)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(e_weights(f_index)): " << abs(e_weights(f_index)) << endl;
			return false;
		}
	}
	#endif /* MDEBUG */

	weights = vector<double>(inputs[0].size());
	for (int f_index = 0; f_index < (int)inputs[0].size(); f_index++) {
		weights[f_index] = e_weights(f_index);
	}

	return true;
}
