#include <chrono>
#include <iostream>
#include <limits>
#include <thread>
#include <random>

#include "action.h"
#include "collapse_network.h"
#include "network.h"
#include "problem.h"
#include "utilities.h"

using namespace std;

default_random_engine generator;

const Action a(0.0, RIGHT);

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ifstream learn_sort_save_file;
	learn_sort_save_file.open("learn_sort_save.txt");
	CollapseNetwork learn_sort(learn_sort_save_file);
	learn_sort_save_file.close();

	ifstream learn_halt_save_file;
	learn_halt_save_file.open("learn_halt_save.txt");
	Network halt_network(learn_halt_save_file);
	learn_halt_save_file.close();

	vector<double> observations;
	observations.push_back(0.0);
	observations.push_back(1.0);
	observations.push_back(2.0);
	observations.push_back(3.0);
	observations.push_back(4.0);
	observations.push_back(5.0);
	observations.push_back(6.0);
	observations.push_back(7.0);
	observations.push_back(8.0);
	observations.push_back(9.0);
	observations.push_back(0.0);

	vector<double> global_vals;

	double state[3] = {};
	vector<vector<double>> time_vals;
	for (int i = 0; i < (int)observations.size(); i++) {
		vector<double> inputs;
		inputs.push_back(state[0]);
		inputs.push_back(state[1]);
		inputs.push_back(state[2]);
		inputs.push_back(observations[i]);
		halt_network.activate(inputs);

		cout << i << ": " << halt_network.val_val->acti_vals[3] << endl;

		state[0] = halt_network.val_val->acti_vals[0];
		state[1] = halt_network.val_val->acti_vals[1];
		state[2] = halt_network.val_val->acti_vals[2];

		vector<double> time_val;
		time_val.push_back(observations[i]);
		time_vals.push_back(time_val);
	}

	vector<double> fan_results = learn_sort.fan((int)observations.size(),
												time_vals,
												global_vals);

	for (int i = 0; i < (int)observations.size(); i++) {
		cout << "fan " << i << ": " << fan_results[i] << endl;
	}

	cout << "Done" << endl;
}