#include "try_instance.h"

#include "constants.h"
#include "try_scope_step.h"

using namespace std;

TryInstance::TryInstance() {
	// do nothing
};

TryInstance::TryInstance(ifstream& input_file) {
	string num_steps_line;
	getline(input_file, num_steps_line);
	int num_steps = stoi(num_steps_line);
	for (int s_index = 0; s_index < num_steps; s_index++) {
		string type_line;
		getline(input_file, type_line);
		this->step_types.push_back(stoi(type_line));

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			string action_line;
			getline(input_file, action_line);
			this->actions.push_back(stoi(action_line));
			this->potential_scopes.push_back(NULL);
		} else {
			this->actions.push_back(-1);
			this->potential_scopes.push_back(new TryScopeStep(input_file));
		}
	}

	{
		string exit_depth_line;
		getline(input_file, exit_depth_line);
		int exit_depth = stoi(exit_depth_line);

		string exit_parent_id_line;
		getline(input_file, exit_parent_id_line);
		int exit_parent_id = stoi(exit_parent_id_line);

		string exit_node_id_line;
		getline(input_file, exit_node_id_line);
		int exit_node_id = stoi(exit_node_id_line);

		this->exit = {exit_depth, {exit_parent_id, exit_node_id}};
	}

	string result_line;
	getline(input_file, result_line);
	this->result = stod(result_line);
}

TryInstance::~TryInstance() {
	for (int s_index = 0; s_index < (int)this->potential_scopes.size(); s_index++) {
		if (this->potential_scopes[s_index] != NULL) {
			delete this->potential_scopes[s_index];
		}
	}
}

void TryInstance::save(ofstream& output_file) {
	output_file << this->step_types.size() << endl;
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		output_file << this->step_types[s_index] << endl;

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			output_file << this->actions[s_index] << endl;
		} else {
			this->potential_scopes[s_index]->save(output_file);
		}
	}

	output_file << this->exit.first << endl;
	output_file << this->exit.second.first << endl;
	output_file << this->exit.second.second << endl;

	output_file << this->result << endl;
}
