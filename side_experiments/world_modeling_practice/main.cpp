// - there are situations where impossible to tell whether made it or not

// - so identity still not fully reliable

// - so loop can fail
// - and identity check can also fail
//   - also, identity check has to be done with very few samples?

// - so state updates can fail
//   - and predictions will be poor
//     - goal is to make the most accurate predictions
//       - so can filter out unnecessary/duplicate data

// - so still have probability of being in multiple states

// - use identity to build map across many samples/runs
//   - but on an individual run, have probability of being in multiple states
// - use identity to understand actions
//   - what stays constant, what changes

// - on new obs, need to do forward backward stuff
//  - use new information to retroactively update everything else

// - don't assign locations/probabilities until need it
//   - then just use max probability
// - so don't track partial states

// - so if taking step by step, and trying to constantly predict next
//   - predict max probability
// - but then on next recalculate everything

// - practice assigning + calculating probabilities

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "model_helpers.h"
#include "network.h"
#include "small.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSmall();

	vector<int> best_init_actions;
	vector<int> best_identity_actions;
	vector<Network*> best_predict_networks;

	double best_ratio = 1.0;
	for (int try_index = 0; try_index < 10; try_index++) {
		vector<int> init_actions;
		vector<int> identity_actions;
		vector<Network*> predict_networks;
		double identity_misguess;
		double explore_misguess;
		init_recognition_helper(problem_type,
								init_actions,
								identity_actions,
								predict_networks,
								identity_misguess,
								explore_misguess);

		double curr_ratio = identity_misguess / explore_misguess;
		if (curr_ratio < best_ratio) {
			best_init_actions = init_actions;
			best_identity_actions = identity_actions;
			for (int n_index = 0; n_index < (int)best_predict_networks.size(); n_index++) {
				delete best_predict_networks[n_index];
			}
			best_predict_networks = predict_networks;
			best_ratio = curr_ratio;
		}
	}

	cout << "best_init_actions:";
	for (int a_index = 0; a_index < (int)best_init_actions.size(); a_index++) {
		cout << " " << best_init_actions[a_index];
	}
	cout << endl;
	cout << "best_identity_actions:";
	for (int a_index = 0; a_index < (int)best_identity_actions.size(); a_index++) {
		cout << " " << best_identity_actions[a_index];
	}
	cout << endl;
	cout << "best_ratio: " << best_ratio << endl;

	{
		ofstream output_file;
		output_file.open("saves/main.txt");

		output_file << best_init_actions.size() << endl;
		for (int a_index = 0; a_index < (int)best_init_actions.size(); a_index++) {
			output_file << best_init_actions[a_index] << endl;
		}

		output_file << best_identity_actions.size() << endl;
		for (int a_index = 0; a_index < (int)best_identity_actions.size(); a_index++) {
			output_file << best_identity_actions[a_index] << endl;
		}

		output_file << best_predict_networks.size() << endl;
		for (int n_index = 0; n_index < (int)best_predict_networks.size(); n_index++) {
			best_predict_networks[n_index]->save(output_file);
		}

		output_file.close();
	}

	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/main.txt");

	// 	string best_init_actions_size_line;
	// 	getline(input_file, best_init_actions_size_line);
	// 	int best_init_actions_size = stoi(best_init_actions_size_line);
	// 	for (int a_index = 0; a_index < best_init_actions_size; a_index++) {
	// 		string action_line;
	// 		getline(input_file, action_line);
	// 		best_init_actions.push_back(stoi(action_line));
	// 	}

	// 	string best_identity_actions_size_line;
	// 	getline(input_file, best_identity_actions_size_line);
	// 	int best_identity_actions_size = stoi(best_identity_actions_size_line);
	// 	for (int a_index = 0; a_index < best_identity_actions_size; a_index++) {
	// 		string action_line;
	// 		getline(input_file, action_line);
	// 		best_identity_actions.push_back(stoi(action_line));
	// 	}

	// 	string best_predict_networks_size_line;
	// 	getline(input_file, best_predict_networks_size_line);
	// 	int best_predict_networks_size = stoi(best_predict_networks_size_line);
	// 	for (int n_index = 0; n_index < best_predict_networks_size; n_index++) {
	// 		best_predict_networks.push_back(new Network(input_file));
	// 	}

	// 	input_file.close();
	// }

	vector<int> return_actions;

	// init_return_helper(problem_type,
	// 				   best_init_actions,
	// 				   best_identity_actions,
	// 				   best_predict_networks,
	// 				   return_actions);

	// cout << "return_actions:";
	// for (int a_index = 0; a_index < (int)return_actions.size(); a_index++) {
	// 	cout << " " << return_actions[a_index];
	// }
	// cout << endl;

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/main.txt");

	// 	output_file << best_init_actions.size() << endl;
	// 	for (int a_index = 0; a_index < (int)best_init_actions.size(); a_index++) {
	// 		output_file << best_init_actions[a_index] << endl;
	// 	}

	// 	output_file << best_identity_actions.size() << endl;
	// 	for (int a_index = 0; a_index < (int)best_identity_actions.size(); a_index++) {
	// 		output_file << best_identity_actions[a_index] << endl;
	// 	}

	// 	output_file << best_predict_networks.size() << endl;
	// 	for (int n_index = 0; n_index < (int)best_predict_networks.size(); n_index++) {
	// 		best_predict_networks[n_index]->save(output_file);
	// 	}

	// 	output_file << return_actions.size() << endl;
	// 	for (int a_index = 0; a_index < (int)return_actions.size(); a_index++) {
	// 		output_file << return_actions[a_index] << endl;
	// 	}

	// 	output_file.close();
	// }

	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/main.txt");

	// 	string best_init_actions_size_line;
	// 	getline(input_file, best_init_actions_size_line);
	// 	int best_init_actions_size = stoi(best_init_actions_size_line);
	// 	for (int a_index = 0; a_index < best_init_actions_size; a_index++) {
	// 		string action_line;
	// 		getline(input_file, action_line);
	// 		best_init_actions.push_back(stoi(action_line));
	// 	}

	// 	string best_identity_actions_size_line;
	// 	getline(input_file, best_identity_actions_size_line);
	// 	int best_identity_actions_size = stoi(best_identity_actions_size_line);
	// 	for (int a_index = 0; a_index < best_identity_actions_size; a_index++) {
	// 		string action_line;
	// 		getline(input_file, action_line);
	// 		best_identity_actions.push_back(stoi(action_line));
	// 	}

	// 	string best_predict_networks_size_line;
	// 	getline(input_file, best_predict_networks_size_line);
	// 	int best_predict_networks_size = stoi(best_predict_networks_size_line);
	// 	for (int n_index = 0; n_index < best_predict_networks_size; n_index++) {
	// 		best_predict_networks.push_back(new Network(input_file));
	// 	}

	// 	string return_actions_size_line;
	// 	getline(input_file, return_actions_size_line);
	// 	int return_actions_size = stoi(return_actions_size_line);
	// 	for (int a_index = 0; a_index < return_actions_size; a_index++) {
	// 		string action_line;
	// 		getline(input_file, action_line);
	// 		return_actions.push_back(stoi(action_line));
	// 	}

	// 	input_file.close();
	// }

	// Network* success_network;
	// init_return_success_helper(problem_type,
	// 						   best_init_actions,
	// 						   best_identity_actions,
	// 						   best_predict_networks,
	// 						   return_actions,
	// 						   success_network);

	delete problem_type;

	cout << "Done" << endl;
}
