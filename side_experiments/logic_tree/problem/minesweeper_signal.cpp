#include "minesweeper_signal.h"

#include <fstream>
#include <iostream>

#include "globals.h"

using namespace std;

const int SAMPLES_NUM_SAVE = 1000;
const int OBS_SIZE = 25;

MinesweeperSignal::MinesweeperSignal() {
	ifstream input_file;
	input_file.open("saves/samples.txt");

	string num_timesteps_line;
	getline(input_file, num_timesteps_line);
	int num_timesteps = stoi(num_timesteps_line);
	for (int t_index = 0; t_index < num_timesteps; t_index++) {
		this->signal_pre_obs.push_back(vector<vector<double>>());
		this->signal_post_obs.push_back(vector<vector<double>>());
		this->signal_scores.push_back(vector<double>());
		this->explore_pre_obs.push_back(vector<vector<double>>());
		this->explore_post_obs.push_back(vector<vector<double>>());
		this->explore_scores.push_back(vector<double>());

		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			this->signal_pre_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->signal_pre_obs.back().back().push_back(stod(obs_line));
			}

			this->signal_post_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->signal_post_obs.back().back().push_back(stod(obs_line));
			}

			string score_line;
			getline(input_file, score_line);
			this->signal_scores.back().push_back(stod(score_line));
		}

		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			this->explore_pre_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->explore_pre_obs.back().back().push_back(stod(obs_line));
			}

			this->explore_post_obs.back().push_back(vector<double>());
			for (int i_index = 0; i_index < OBS_SIZE; i_index++) {
				string obs_line;
				getline(input_file, obs_line);
				this->explore_post_obs.back().back().push_back(stod(obs_line));
			}

			string score_line;
			getline(input_file, score_line);
			this->explore_scores.back().push_back(stod(score_line));
		}

		cout << "loaded " << t_index << endl;
	}

	input_file.close();
}

void MinesweeperSignal::get_instance(std::vector<double>& obs,
									 double& target_val) {
	uniform_int_distribution<int> timestamp_distribution(0, this->signal_pre_obs.size()-1);
	int timestamp = timestamp_distribution(generator);
	uniform_int_distribution<int> sample_distribution(0, SAMPLES_NUM_SAVE-1);
	int sample = sample_distribution(generator);

	obs.insert(obs.end(), this->signal_pre_obs[timestamp][sample].begin(), this->signal_pre_obs[timestamp][sample].end());
	obs.insert(obs.end(), this->signal_post_obs[timestamp][sample].begin(), this->signal_post_obs[timestamp][sample].end());

	target_val = this->signal_scores[timestamp][sample];
}
