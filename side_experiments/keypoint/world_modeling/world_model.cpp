#include "world_model.h"

#include "keypoint.h"

using namespace std;

WorldModel::WorldModel() {
	// do nothing
}

WorldModel::~WorldModel() {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		delete this->keypoints[k_index];
	}
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->keypoints.size() << endl;
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		this->keypoints[k_index]->save(output_file);
	}

	output_file << this->path_actions.size() << endl;
	for (int p_index = 0; p_index < (int)this->path_actions.size(); p_index++) {
		output_file << this->path_actions[p_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->path_actions[p_index].size(); a_index++) {
			output_file << this->path_actions[p_index][a_index] << endl;
		}

		output_file << this->path_obs[p_index].size() << endl;
		for (int o_index = 0; o_index < (int)this->path_obs[p_index].size(); o_index++) {
			output_file << this->path_obs[p_index][o_index] << endl;
		}

		output_file << this->path_start_indexes[p_index] << endl;

		output_file << this->path_end_indexes[p_index] << endl;

		output_file << this->path_success_likelihoods[p_index] << endl;
	}
}

void WorldModel::load(ifstream& input_file) {
	string num_keypoints_line;
	getline(input_file, num_keypoints_line);
	int num_keypoints = stoi(num_keypoints_line);
	for (int k_index = 0; k_index < num_keypoints; k_index++) {
		Keypoint* keypoint = new Keypoint();
		keypoint->load(input_file);
		this->keypoints.push_back(keypoint);
	}

	string num_paths_line;
	getline(input_file, num_paths_line);
	int num_paths = stoi(num_paths_line);
	for (int p_index = 0; p_index < num_paths; p_index++) {
		string obs_size_line;
		getline(input_file, obs_size_line);
		int obs_size = stoi(obs_size_line);
		vector<double> obs;
		for (int o_index = 0; o_index < obs_size; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stoi(obs_line));
		}
		this->path_obs.push_back(obs);

		string actions_size_line;
		getline(input_file, actions_size_line);
		int actions_size = stoi(actions_size_line);
		vector<int> actions;
		for (int a_index = 0; a_index < actions_size; a_index++) {
			string action_line;
			getline(input_file, action_line);
			actions.push_back(stoi(action_line));
		}
		this->path_actions.push_back(actions);

		string start_index_line;
		getline(input_file, start_index_line);
		this->path_start_indexes.push_back(stoi(start_index_line));

		string end_index_line;
		getline(input_file, end_index_line);
		this->path_end_indexes.push_back(stoi(end_index_line));

		string likelihood_line;
		getline(input_file, likelihood_line);
		this->path_success_likelihoods.push_back(stod(likelihood_line));
	}
}
