#include "pattern.h"

#include "network.h"

using namespace std;

Pattern::Pattern(vector<int> actions,
				 vector<int> keypoints,
				 vector<double> keypoint_averages,
				 vector<double> keypoint_standard_deviations,
				 vector<int> inputs,
				 Network* network) {
	this->actions = actions;
	this->keypoints = keypoints;
	this->keypoint_averages = keypoint_averages;
	this->keypoint_standard_deviations = keypoint_standard_deviations;
	this->inputs = inputs;
	this->network = network;
}

Pattern::Pattern(ifstream& input_file) {
	string num_actions_line;
	getline(input_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		string action_line;
		getline(input_file, action_line);
		this->actions.push_back(stoi(action_line));
	}

	string num_keypoints_line;
	getline(input_file, num_keypoints_line);
	int num_keypoints = stoi(num_keypoints_line);
	for (int k_index = 0; k_index < num_keypoints; k_index++) {
		string index_line;
		getline(input_file, index_line);
		this->keypoints.push_back(stoi(index_line));

		string average_line;
		getline(input_file, average_line);
		this->keypoint_averages.push_back(stod(average_line));

		string standard_deviation_line;
		getline(input_file, standard_deviation_line);
		this->keypoint_standard_deviations.push_back(stod(standard_deviation_line));
	}

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->inputs.push_back(stoi(index_line));
	}

	this->network = new Network(input_file);
}

Pattern::~Pattern() {
	delete this->network;
}

void Pattern::save(ofstream& output_file) {
	output_file << this->actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		output_file << this->actions[a_index] << endl;
	}

	output_file << this->keypoints.size() << endl;
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		output_file << this->keypoints[k_index] << endl;
		output_file << this->keypoint_averages[k_index] << endl;
		output_file << this->keypoint_standard_deviations[k_index] << endl;
	}

	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		output_file << this->inputs[i_index] << endl;
	}

	this->network->save(output_file);
}
