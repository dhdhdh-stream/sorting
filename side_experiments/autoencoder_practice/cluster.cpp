// TODO: not good enough
// - perhaps need specific clustering
// - maybe can discard hard problems

// TODO: try removing duplicates

// TODO: actually, don't have specific eval sequence
// - just use normal sequence
//   - train on entire with 10% dropout, then use to evaluate new
// - idea is stuff can happen in-between, but as long as see things in the end that you want to see, then good
// - need to train to maximize both score and information
// - end needs to have enough information to evaluate
//   - so don't explore unless have significant information
//     - or use outer layer?
//       - but then not general, so don't

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "autoencoder.h"
#include "boltzmann.h"
#include "network.h"

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

	Boltzmann* boltzmann = new Boltzmann();

	uniform_int_distribution<int> x_distribution(0, 4);
	uniform_int_distribution<int> y_distribution(0, 4);
	double sum_error = 0.0;
	for (int iter_index = 0; iter_index < 500000; iter_index++) {
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

	vector<Network*> networks(16);
	for (int n_index = 0; n_index < 16; n_index++) {
		networks[n_index] = new Network();
		for (int i_index = 0; i_index < 18; i_index++) {
			networks[n_index]->input->acti_vals.push_back(0.0);
			networks[n_index]->input->errors.push_back(0.0);
		}
		for (int i_index = 0; i_index < 20; i_index++) {
			networks[n_index]->hiddens[0]->acti_vals.push_back(0.0);
			networks[n_index]->hiddens[0]->errors.push_back(0.0);
		}
		networks[n_index]->hiddens[0]->update_structure();
		networks[n_index]->output->update_structure();
	}


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

		int network_index = boltzmann->classify(obs);

		networks[network_index]->activate(obs);

		sum_error += (distance - networks[network_index]->output->acti_vals[0]) * (distance - networks[network_index]->output->acti_vals[0]);

		double error = distance - networks[network_index]->output->acti_vals[0];

		networks[network_index]->backprop(error);

		if (iter_index % 10000 == 0) {
			cout << "network_index: " << network_index << endl;

			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	cout << "Done" << endl;
}
