#include "try_instance.h"

#include "constants.h"
#include "try_scope_step.h"

using namespace std;

TryInstance::TryInstance() {
	// do nothing
};

TryInstance::TryInstance(ifstream& input_file) {
	{
		string start_context_size_line;
		getline(input_file, start_context_size_line);
		int start_context_size = stoi(start_context_size_line);
		vector<int> scope_context;
		vector<int> node_context;
		for (int c_index = 0; c_index < start_context_size; c_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			scope_context.push_back(stoi(scope_id_line));

			string node_id_line;
			getline(input_file, node_id_line);
			node_context.push_back(stoi(node_id_line));
		}
		this->start = {scope_context, node_context};
	}

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
		string exit_context_size_line;
		getline(input_file, exit_context_size_line);
		int exit_context_size = stoi(exit_context_size_line);
		vector<int> scope_context;
		vector<int> node_context;
		for (int c_index = 0; c_index < exit_context_size; c_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			scope_context.push_back(stoi(scope_id_line));

			string node_id_line;
			getline(input_file, node_id_line);
			node_context.push_back(stoi(node_id_line));
		}
		this->exit = {scope_context, node_context};
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
	{
		output_file << this->start.first.size() << endl;
		for (int c_index = 0; c_index < (int)this->start.first.size(); c_index++) {
			output_file << this->start.first[c_index] << endl;
			output_file << this->start.second[c_index] << endl;
		}
	}

	output_file << this->step_types.size() << endl;
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		output_file << this->step_types[s_index] << endl;

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			output_file << this->actions[s_index] << endl;
		} else {
			this->potential_scopes[s_index]->save(output_file);
		}
	}

	{
		output_file << this->exit.first.size() << endl;
		for (int c_index = 0; c_index < (int)this->exit.first.size(); c_index++) {
			output_file << this->exit.first[c_index] << endl;
			output_file << this->exit.second[c_index] << endl;
		}
	}

	output_file << this->result << endl;
}
