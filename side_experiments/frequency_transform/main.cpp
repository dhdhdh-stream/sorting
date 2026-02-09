// TODO: try twin peak
// TODO: try w/ correlation
// TODO: try w/ softmax
// - but with softmax, can no longer linear, and then there's no point

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "globals.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// for (int val = -2; val <= 2; val++) {
	// 	for (int i_index = 0; i_index < 10; i_index++) {
	// 		cout << sin(10.0 * val * i_index) << endl;
	// 	}
	// 	cout << endl;
	// }

	uniform_int_distribution<int> input_distribution(-2, 2);

	// Eigen::MatrixXd inputs(100, 1 + 10);
	// Eigen::MatrixXd inputs(100, 1 + 20);
	// Eigen::MatrixXd inputs(100, 1 + 1);
	Eigen::MatrixXd inputs(100, 1 + 5);
	Eigen::VectorXd outputs(100);
	for (int h_index = 0; h_index < 100; h_index++) {
		int val = input_distribution(generator);

		inputs(h_index, 0) = 1.0;
		// for (int i_index = 0; i_index < 10; i_index++) {
		// for (int i_index = 0; i_index < 20; i_index++) {
		// for (int i_index = 0; i_index < 1; i_index++) {
		for (int i_index = 0; i_index < 5; i_index++) {
			inputs(h_index, 1 + i_index) = sin(val * i_index);
			// inputs(h_index, 1 + i_index) = sin(10.0 * val * i_index);
			// inputs(h_index, 1 + i_index) = sin(0.1 * val * i_index);
			// inputs(h_index, 1 + i_index) = sin(val + i_index);
			// inputs(h_index, 1 + i_index) = sin(10.0 * val + i_index);
		}

		// if (val == 0) {
		if (val == 1) {
			outputs(h_index) = 1.0;
		} else {
			outputs(h_index) = -1.0;
		}
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);

		Eigen::VectorXd predicted = inputs * weights;
		cout << predicted << endl;

		cout << "outputs:" << endl;
		for (int h_index = 0; h_index < (int)outputs.size(); h_index++) {
			cout << outputs[h_index] << endl;
		}

		cout << weights << endl;
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
	}

	cout << "Done" << endl;
}
