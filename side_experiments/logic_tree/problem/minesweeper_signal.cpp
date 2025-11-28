#include "minesweeper_signal.h"

#include <fstream>
#include <iostream>

#include "globals.h"

using namespace std;

const int SAMPLES_NUM_SAVE = 4000;
const int NUM_TRAIN = 3000;
const int NUM_TEST = 1000;
const int OBS_SIZE = 25;

MinesweeperSignal::MinesweeperSignal() {
	ifstream input_file;
	// input_file.open("saves/samples.txt");
	input_file.open("saves/samples_t_1764288560.txt");

	string num_timesteps_line;
	getline(input_file, num_timesteps_line);
	int num_timesteps = stoi(num_timesteps_line);
	for (int t_index = 0; t_index < num_timesteps; t_index++) {
		this->train_signal_pre_obs.push_back(vector<vector<double>>());
		this->train_signal_post_obs.push_back(vector<vector<double>>());
		this->train_signal_scores.push_back(vector<double>());
		this->train_explore_pre_obs.push_back(vector<vector<double>>());
		this->train_explore_post_obs.push_back(vector<vector<double>>());
		this->train_explore_scores.push_back(vector<double>());

		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			this->train_signal_pre_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->train_signal_pre_obs.back().back().push_back(stod(obs_line));
			}

			this->train_signal_post_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->train_signal_post_obs.back().back().push_back(stod(obs_line));
			}

			string score_line;
			getline(input_file, score_line);
			this->train_signal_scores.back().push_back(stod(score_line));
		}

		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			this->train_explore_pre_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->train_explore_pre_obs.back().back().push_back(stod(obs_line));
			}

			this->train_explore_post_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->train_explore_post_obs.back().back().push_back(stod(obs_line));
			}

			string score_line;
			getline(input_file, score_line);
			this->train_explore_scores.back().push_back(stod(score_line));
		}

		cout << "loaded " << t_index << endl;
	}

	for (int t_index = 0; t_index < num_timesteps; t_index++) {
		this->test_signal_pre_obs.push_back(vector<vector<double>>());
		this->test_signal_post_obs.push_back(vector<vector<double>>());
		this->test_signal_scores.push_back(vector<double>());
		this->test_explore_pre_obs.push_back(vector<vector<double>>());
		this->test_explore_post_obs.push_back(vector<vector<double>>());
		this->test_explore_scores.push_back(vector<double>());

		while (this->train_signal_pre_obs[t_index].size() > NUM_TRAIN) {
			uniform_int_distribution<int> distribution(0, this->train_signal_pre_obs[t_index].size()-1);
			int index = distribution(generator);
			this->test_signal_pre_obs.back().push_back(this->train_signal_pre_obs[t_index][index]);
			this->train_signal_pre_obs[t_index].erase(this->train_signal_pre_obs[t_index].begin() + index);
			this->test_signal_post_obs.back().push_back(this->train_signal_post_obs[t_index][index]);
			this->train_signal_post_obs[t_index].erase(this->train_signal_post_obs[t_index].begin() + index);
			this->test_signal_scores.back().push_back(this->train_signal_scores[t_index][index]);
			this->train_signal_scores[t_index].erase(this->train_signal_scores[t_index].begin() + index);
		}

		while (this->train_explore_pre_obs[t_index].size() > NUM_TRAIN) {
			uniform_int_distribution<int> distribution(0, this->train_explore_pre_obs[t_index].size()-1);
			int index = distribution(generator);
			this->test_explore_pre_obs.back().push_back(this->train_explore_pre_obs[t_index][index]);
			this->train_explore_pre_obs[t_index].erase(this->train_explore_pre_obs[t_index].begin() + index);
			this->test_explore_post_obs.back().push_back(this->train_explore_post_obs[t_index][index]);
			this->train_explore_post_obs[t_index].erase(this->train_explore_post_obs[t_index].begin() + index);
			this->test_explore_scores.back().push_back(this->train_explore_scores[t_index][index]);
			this->train_explore_scores[t_index].erase(this->train_explore_scores[t_index].begin() + index);
		}
	}

	input_file.close();
}

void MinesweeperSignal::get_train_instance(std::vector<double>& obs,
										   double& target_val) {
	uniform_int_distribution<int> timestamp_distribution(0, this->train_signal_pre_obs.size()-1);
	int timestamp = timestamp_distribution(generator);
	uniform_int_distribution<int> sample_distribution(0, NUM_TRAIN-1);
	int sample = sample_distribution(generator);

	obs.insert(obs.end(), this->train_signal_pre_obs[timestamp][sample].begin(), this->train_signal_pre_obs[timestamp][sample].end());
	obs.insert(obs.end(), this->train_signal_post_obs[timestamp][sample].begin(), this->train_signal_post_obs[timestamp][sample].end());

	target_val = this->train_signal_scores[timestamp][sample];

	// uniform_int_distribution<int> timestamp_distribution(0, this->train_explore_pre_obs.size()-1);
	// int timestamp = timestamp_distribution(generator);
	// uniform_int_distribution<int> sample_distribution(0, NUM_TRAIN-1);
	// int sample = sample_distribution(generator);

	// obs.insert(obs.end(), this->train_explore_pre_obs[timestamp][sample].begin(), this->train_explore_pre_obs[timestamp][sample].end());
	// obs.insert(obs.end(), this->train_explore_post_obs[timestamp][sample].begin(), this->train_explore_post_obs[timestamp][sample].end());

	// target_val = this->train_explore_scores[timestamp][sample];
}

void MinesweeperSignal::get_test_instance(std::vector<double>& obs,
										  double& target_val) {
	uniform_int_distribution<int> timestamp_distribution(0, this->test_signal_pre_obs.size()-1);
	int timestamp = timestamp_distribution(generator);
	uniform_int_distribution<int> sample_distribution(0, NUM_TEST-1);
	int sample = sample_distribution(generator);

	obs.insert(obs.end(), this->test_signal_pre_obs[timestamp][sample].begin(), this->test_signal_pre_obs[timestamp][sample].end());
	obs.insert(obs.end(), this->test_signal_post_obs[timestamp][sample].begin(), this->test_signal_post_obs[timestamp][sample].end());

	target_val = this->test_signal_scores[timestamp][sample];

	// uniform_int_distribution<int> timestamp_distribution(0, this->test_explore_pre_obs.size()-1);
	// int timestamp = timestamp_distribution(generator);
	// uniform_int_distribution<int> sample_distribution(0, NUM_TEST-1);
	// int sample = sample_distribution(generator);

	// obs.insert(obs.end(), this->test_explore_pre_obs[timestamp][sample].begin(), this->test_explore_pre_obs[timestamp][sample].end());
	// obs.insert(obs.end(), this->test_explore_post_obs[timestamp][sample].begin(), this->test_explore_post_obs[timestamp][sample].end());

	// target_val = this->test_explore_scores[timestamp][sample];
}
