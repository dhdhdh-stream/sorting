#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ifstream input_file("saves/data_1771181889.txt");

	string num_existing_line;
	getline(input_file, num_existing_line);
	int num_existing = stoi(num_existing_line);
	vector<vector<double>> existing_obs_histories;
	vector<double> existing_target_val_histories;
	for (int h_index = 0; h_index < num_existing; h_index++) {
		vector<double> curr_obs;
		for (int i_index = 0; i_index < 50; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			curr_obs.push_back(stod(obs_line));
		}
		existing_obs_histories.push_back(curr_obs);

		string target_val_line;
		getline(input_file, target_val_line);
		existing_target_val_histories.push_back(stod(target_val_line));
	}

	string num_explore_line;
	getline(input_file, num_explore_line);
	int num_explore = stoi(num_explore_line);
	vector<vector<double>> explore_obs_histories;
	vector<double> explore_target_val_histories;
	for (int h_index = 0; h_index < num_explore; h_index++) {
		vector<double> curr_obs;
		for (int i_index = 0; i_index < 50; i_index++) {
			string obs_line;
			getline(input_file, obs_line);
			curr_obs.push_back(stod(obs_line));
		}
		explore_obs_histories.push_back(curr_obs);

		string target_val_line;
		getline(input_file, target_val_line);
		explore_target_val_histories.push_back(stod(target_val_line));
	}

	input_file.close();

	for (int h_index = 0; h_index < 40; h_index++) {
		cout << "pre_obs:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << existing_obs_histories[h_index][5 * y_index + x_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << existing_obs_histories[h_index][25 + 5 * y_index + x_index] << " ";
			}
			cout << endl;
		}
		cout << "existing_target_val_histories[h_index]: " << existing_target_val_histories[h_index] << endl;
		cout << endl;
	}

	// for (int h_index = 0; h_index < 20; h_index++) {
	// 	cout << "pre_obs:" << endl;
	// 	for (int x_index = 0; x_index < 5; x_index++) {
	// 		for (int y_index = 0; y_index < 5; y_index++) {
	// 			cout << explore_obs_histories[h_index][5 * y_index + x_index] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	cout << "post_obs:" << endl;
	// 	for (int x_index = 0; x_index < 5; x_index++) {
	// 		for (int y_index = 0; y_index < 5; y_index++) {
	// 			cout << explore_obs_histories[h_index][25 + 5 * y_index + x_index] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	cout << "explore_target_val_histories[h_index]: " << explore_target_val_histories[h_index] << endl;
	// 	cout << endl;
	// }

	cout << "Done" << endl;
}
