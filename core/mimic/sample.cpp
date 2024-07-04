#include "sample.h"

#include <fstream>

#include "globals.h"
#include "problem.h"

using namespace std;

Sample::Sample() {
	// do nothing
}

Sample::Sample(int id) {
	this->id = id;

	ifstream input_file;
	input_file.open("saves/samples/" + to_string(this->id) + ".txt");

	string num_steps_line;
	getline(input_file, num_steps_line);
	int num_steps = stoi(num_steps_line);
	for (int s_index = 0; s_index < num_steps; s_index++) {
		this->actions.push_back(Action(input_file));

		vector<double> step_obs(problem_type->num_obs());
		for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			step_obs[o_index] = stod(obs_line);
		}
		this->obs.push_back(step_obs);
	}

	string result_line;
	getline(input_file, result_line);
	this->result = stod(result_line);

	input_file.close();
}

void Sample::save() {
	ofstream output_file;
	output_file.open("saves/samples/" + to_string(this->id) + ".txt");

	output_file << this->actions.size() << endl;
	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		this->actions[s_index].save(output_file);

		for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
			output_file << this->obs[s_index][o_index] << endl;
		}
	}

	output_file << this->result << endl;

	output_file.close();
}