#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "autoencoder.h"
#include "boltzmann.h"

using namespace std;

int seed;

default_random_engine generator;

double obs_helper(int obj_x,
				  int obj_y,
				  int loc_x,
				  int loc_y) {
	if (loc_x < 0 || loc_x > 4 || loc_y < 0 || loc_y > 4) {
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

	// Autoencoder* autoencoder = new Autoencoder();
	Boltzmann* boltzmann = new Boltzmann();

	uniform_int_distribution<int> x_distribution(0, 4);
	uniform_int_distribution<int> y_distribution(0, 4);
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

		// autoencoder->activate(obs);

		// vector<double> errors(18);
		// for (int i_index = 0; i_index < 18; i_index++) {
		// 	sum_error += (obs[i_index] - autoencoder->output->acti_vals[i_index]) * (obs[i_index] - autoencoder->output->acti_vals[i_index]);
		// 	errors[i_index] = obs[i_index] - autoencoder->output->acti_vals[i_index];
		// }

		// autoencoder->backprop(errors);

		// if (iter_index % 10000 == 0) {
		// 	for (int i_index = 0; i_index < 18; i_index++) {
		// 		cout << obs[i_index] << " " << autoencoder->output->acti_vals[i_index] << endl;
		// 	}

		// 	cout << iter_index << endl;
		// 	cout << "sum_error: " << sum_error << endl;
		// 	sum_error = 0.0;
		// }

		boltzmann->activate(obs);

		vector<double> errors(18);
		for (int i_index = 0; i_index < 18; i_index++) {
			sum_error += (obs[i_index] - boltzmann->output->acti_vals[i_index]) * (obs[i_index] - boltzmann->output->acti_vals[i_index]);
			errors[i_index] = obs[i_index] - boltzmann->output->acti_vals[i_index];
		}

		boltzmann->backprop(errors);

		if (iter_index % 10000 == 0) {
			for (int i_index = 0; i_index < 18; i_index++) {
				cout << obs[i_index] << " " << boltzmann->output->acti_vals[i_index] << endl;
			}

			cout << "boltzmann->hidden:" << endl;
			for (int n_index = 0; n_index < (int)boltzmann->hidden->acti_vals.size(); n_index++) {
				cout << n_index << ": " << boltzmann->hidden->acti_vals[n_index] << " " << boltzmann->hidden->sum_vals[n_index] << endl;
			}

			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	cout << "Done" << endl;
}
