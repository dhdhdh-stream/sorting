#include "solution.h"

#include "action_network.h"
#include "constants.h"
#include "obs_network.h"
#include "score_network.h"

using namespace std;

Solution::Solution(int num_obs,
				   int num_actions) {
	this->obs_network = new ObsNetwork(NUM_STATES,
									   num_obs);

	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->action_networks.push_back(new ActionNetwork(NUM_STATES));
	}

	this->score_network = new ScoreNetwork(NUM_STATES);
}

Solution::Solution(string path,
				   string name) {
	ifstream input_file;
	input_file.open(path + name);

	this->obs_network = new ObsNetwork(input_file);

	string num_actions_line;
	getline(input_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->action_networks.push_back(new ActionNetwork(input_file));
	}

	this->score_network = new ScoreNetwork(input_file);

	input_file.close();
}

Solution::~Solution() {
	delete this->obs_network;

	for (int a_index = 0; a_index < (int)this->action_networks.size(); a_index++) {
		delete this->action_networks[a_index];
	}

	delete this->score_network;
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->obs_network->save(output_file);

	output_file << this->action_networks.size() << endl;
	for (int a_index = 0; a_index < (int)this->action_networks.size(); a_index++) {
		this->action_networks[a_index]->save(output_file);
	}

	this->score_network->save(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
