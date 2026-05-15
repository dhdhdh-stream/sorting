#include "world_model_wrapper.h"

#include "problem.h"
#include "world_model.h"

using namespace std;

WorldModelWrapper::WorldModelWrapper(ProblemType* problem_type) {
	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	this->world_model = new WorldModel();
}

WorldModelWrapper::WorldModelWrapper(std::string path,
									 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	this->num_actions = stoi(num_actions_line);

	this->world_model = new WorldModel();
	this->world_model->load(input_file);

	string num_samples_line;
	getline(input_file, num_samples_line);
	int num_samples = stoi(num_samples_line);
	for (int s_index = 0; s_index < num_samples; s_index++) {
		string obs_num_steps_line;
		getline(input_file, obs_num_steps_line);
		int obs_num_steps = stoi(obs_num_steps_line);
		vector<vector<double>> run_obs;
		for (int a_index = 0; a_index < obs_num_steps; a_index++) {
			vector<double> step_obs;
			for (int o_index = 0; o_index < this->num_obs; o_index++) {
				string obs_line;
				getline(input_file, obs_line);
				step_obs.push_back(stod(obs_line));
			}
			run_obs.push_back(step_obs);
		}
		this->sample_obs.push_back(run_obs);

		string action_num_steps_line;
		getline(input_file, action_num_steps_line);
		int action_num_steps = stoi(action_num_steps_line);
		vector<int> run_actions;
		for (int a_index = 0; a_index < action_num_steps; a_index++) {
			string action_line;
			getline(input_file, action_line);
			run_actions.push_back(stoi(action_line));
		}
		this->sample_actions.push_back(run_actions);

		string target_val_line;
		getline(input_file, target_val_line);
		this->sample_target_vals.push_back(stod(target_val_line));
	}

	input_file.close();
}

WorldModelWrapper::~WorldModelWrapper() {
	delete this->world_model;
}

void WorldModelWrapper::save(string path,
							 string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->num_obs << endl;
	output_file << this->num_actions << endl;

	this->world_model->save(output_file);

	output_file << this->sample_obs.size() << endl;
	for (int s_index = 0; s_index < (int)this->sample_obs.size(); s_index++) {
		output_file << this->sample_obs[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->sample_obs[s_index].size(); a_index++) {
			for (int o_index = 0; o_index < this->num_obs; o_index++) {
				output_file << this->sample_obs[s_index][a_index][o_index] << endl;
			}
		}

		output_file << this->sample_actions[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->sample_actions[s_index].size(); a_index++) {
			output_file << this->sample_actions[s_index][a_index] << endl;
		}

		output_file << this->sample_target_vals[s_index] << endl;
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
