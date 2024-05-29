#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

double obs_helper(int obj_x,
				  int obj_y,
				  int loc_x,
				  int loc_y) {
	if (loc_x < 0 || loc_x > 4 || loc_y < 0 || loc_y > 4) {
	// if (loc_x < 0 || loc_x > 2 || loc_y < 0 || loc_y > 2) {
		return -1.0;
	} else if (loc_x == obj_x
			&& loc_y == obj_y) {
		return 1.0;
	} else {
		return 0.0;
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* network = new Network();
	for (int i_index = 0; i_index < 18; i_index++) {
		network->input->acti_vals.push_back(0.0);
		network->input->errors.push_back(0.0);
	}
	for (int i_index = 0; i_index < 40; i_index++) {
		network->hiddens[0]->acti_vals.push_back(0.0);
		network->hiddens[0]->errors.push_back(0.0);
	}
	network->hiddens[0]->update_structure();
	network->output->update_structure();

	uniform_int_distribution<int> x_distribution(0, 4);
	// uniform_int_distribution<int> x_distribution(0, 2);
	uniform_int_distribution<int> y_distribution(0, 4);
	// uniform_int_distribution<int> y_distribution(0, 2);
	double sum_error = 0.0;
	for (int iter_index = 0; iter_index < 2000000; iter_index++) {
		int start_obj_x = x_distribution(generator);
		int start_obj_y = y_distribution(generator);

		int start_loc_x = x_distribution(generator);
		int start_loc_y = y_distribution(generator);

		int end_obj_x = x_distribution(generator);
		int end_obj_y = y_distribution(generator);

		int end_loc_x = x_distribution(generator);
		int end_loc_y = y_distribution(generator);

		double x_distance = end_obj_x - start_obj_x;
		double y_distance = end_obj_y - start_obj_y;

		double distance = sqrt(x_distance * x_distance + y_distance * y_distance);

		vector<double> obs;

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x-1,
					  start_loc_y+1));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x,
					  start_loc_y+1));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x+1,
					  start_loc_y+1));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x-1,
					  start_loc_y));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x,
					  start_loc_y));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x+1,
					  start_loc_y));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x-1,
					  start_loc_y-1));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x,
					  start_loc_y-1));

		obs.push_back(obs_helper(start_obj_x,
					  start_obj_y,
					  start_loc_x+1,
					  start_loc_y-1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x-1,
					  end_loc_y+1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x,
					  end_loc_y+1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x+1,
					  end_loc_y+1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x-1,
					  end_loc_y));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x,
					  end_loc_y));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x+1,
					  end_loc_y));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x-1,
					  end_loc_y-1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x,
					  end_loc_y-1));

		obs.push_back(obs_helper(end_obj_x,
					  end_obj_y,
					  end_loc_x+1,
					  end_loc_y-1));

		network->activate(obs);

		sum_error += (distance - network->output->acti_vals[0]) * (distance - network->output->acti_vals[0]);

		double error = distance - network->output->acti_vals[0];

		network->backprop(error);

		if (iter_index % 10000 == 0) {
			cout << "start_obj_x: " << start_obj_x << endl;
			cout << "start_obj_y: " << start_obj_y << endl;

			cout << "start_loc_x: " << start_loc_x << endl;
			cout << "start_loc_y: " << start_loc_y << endl;

			cout << "end_obj_x: " << end_obj_x << endl;
			cout << "end_obj_y: " << end_obj_y << endl;

			cout << "end_loc_x: " << end_loc_x << endl;
			cout << "end_loc_y: " << end_loc_y << endl;

			cout << "distance: " << distance << endl;

			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	cout << "Done" << endl;
}
