#include "wrapper.h"

#include "obs_node.h"
#include "problem.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

Wrapper::Wrapper(ProblemType* problem_type) {
	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	this->world_model = new WorldModel();

	this->solution = new Solution();

	this->solution->timestamp = 0;
	this->solution->curr_score = 0.0;

	this->solution->node_counter = 0;

	ObsNode* start_node = new ObsNode();
	start_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[start_node->id] = start_node;
	start_node->next_node_id = -1;
	start_node->next_node = NULL;

	this->solution->score_index = 0;

	this->iter = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::Wrapper(std::string path,
				 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	this->num_actions = stoi(num_actions_line);

	string num_old_samples_line;
	getline(input_file, num_old_samples_line);
	int num_old_samples = stoi(num_old_samples_line);
	for (int s_index = 0; s_index < num_old_samples; s_index++) {
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
		this->old_sample_obs.push_back(run_obs);

		string action_num_steps_line;
		getline(input_file, action_num_steps_line);
		int action_num_steps = stoi(action_num_steps_line);
		vector<int> run_actions;
		for (int a_index = 0; a_index < action_num_steps; a_index++) {
			string action_line;
			getline(input_file, action_line);
			run_actions.push_back(stoi(action_line));
		}
		this->old_sample_actions.push_back(run_actions);

		string target_val_line;
		getline(input_file, target_val_line);
		this->old_sample_target_vals.push_back(stod(target_val_line));
	}

	string num_new_samples_line;
	getline(input_file, num_new_samples_line);
	int num_new_samples = stoi(num_new_samples_line);
	for (int s_index = 0; s_index < num_new_samples; s_index++) {
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
		this->new_sample_obs.push_back(run_obs);

		string action_num_steps_line;
		getline(input_file, action_num_steps_line);
		int action_num_steps = stoi(action_num_steps_line);
		vector<int> run_actions;
		for (int a_index = 0; a_index < action_num_steps; a_index++) {
			string action_line;
			getline(input_file, action_line);
			run_actions.push_back(stoi(action_line));
		}
		this->new_sample_actions.push_back(run_actions);

		string target_val_line;
		getline(input_file, target_val_line);
		this->new_sample_target_vals.push_back(stod(target_val_line));
	}

	this->world_model = new WorldModel();
	this->world_model->load(input_file);

	this->solution = new Solution();
	this->solution->load(input_file,
						 this);

	input_file.close();

	this->iter = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::~Wrapper() {
	delete this->world_model;
	delete this->solution;
}

void Wrapper::save(string path,
				   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->num_obs << endl;
	output_file << this->num_actions << endl;

	output_file << this->old_sample_obs.size() << endl;
	for (int s_index = 0; s_index < (int)this->old_sample_obs.size(); s_index++) {
		output_file << this->old_sample_obs[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->old_sample_obs[s_index].size(); a_index++) {
			for (int o_index = 0; o_index < this->num_obs; o_index++) {
				output_file << this->old_sample_obs[s_index][a_index][o_index] << endl;
			}
		}

		output_file << this->old_sample_actions[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->old_sample_actions[s_index].size(); a_index++) {
			output_file << this->old_sample_actions[s_index][a_index] << endl;
		}

		output_file << this->old_sample_target_vals[s_index] << endl;
	}

	output_file << this->new_sample_obs.size() << endl;
	for (int s_index = 0; s_index < (int)this->new_sample_obs.size(); s_index++) {
		output_file << this->new_sample_obs[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->new_sample_obs[s_index].size(); a_index++) {
			for (int o_index = 0; o_index < this->num_obs; o_index++) {
				output_file << this->new_sample_obs[s_index][a_index][o_index] << endl;
			}
		}

		output_file << this->new_sample_actions[s_index].size() << endl;
		for (int a_index = 0; a_index < (int)this->new_sample_actions[s_index].size(); a_index++) {
			output_file << this->new_sample_actions[s_index][a_index] << endl;
		}

		output_file << this->new_sample_target_vals[s_index] << endl;
	}

	this->world_model->save(output_file);
	this->solution->save(output_file,
						 this);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void Wrapper::save_for_display(string path,
							   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
