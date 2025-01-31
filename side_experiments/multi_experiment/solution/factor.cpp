#include "factor.h"

#include "network.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Factor::Factor() {
	this->is_used = false;
}

Factor::~Factor() {
	delete this->network;
}

void Factor::clean_inputs(Scope* scope,
						  int node_id) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			if (this->inputs[i_index].first.first[l_index] == scope
					&& this->inputs[i_index].first.second[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}
}

void Factor::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			if (this->inputs[i_index].first.first[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}
}

void Factor::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		output_file << this->inputs[i_index].first.first.size() << endl;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			output_file << this->inputs[i_index].first.first[l_index]->id << endl;
			output_file << this->inputs[i_index].first.second[l_index] << endl;
		}

		output_file << this->inputs[i_index].second.first << endl;
		output_file << this->inputs[i_index].second.second << endl;
	}

	this->network->save(output_file);
}

void Factor::load(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		vector<Scope*> scope_ids;
		vector<int> node_ids;

		string num_layers_line;
		getline(input_file, num_layers_line);
		int num_layers = stoi(num_layers_line);
		for (int l_index = 0; l_index < num_layers; l_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			scope_ids.push_back(solution->scopes[stoi(scope_id_line)]);

			string node_id_line;
			getline(input_file, node_id_line);
			node_ids.push_back(stoi(node_id_line));
		}

		string factor_index_line;
		getline(input_file, factor_index_line);
		int factor_index = stoi(factor_index_line);

		string obs_index_line;
		getline(input_file, obs_index_line);
		int obs_index = stoi(obs_index_line);

		this->inputs.push_back({{scope_ids, node_ids}, {factor_index, obs_index}});
	}

	this->network = new Network(input_file);
}

void Factor::link() {
	if (!this->is_used) {
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			Scope* scope = this->inputs[i_index].first.first.back();
			AbstractNode* node = scope->nodes[this->inputs[i_index].first.second.back()];
			switch (node->type) {
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;

					if (this->inputs[i_index].second.first != -1) {
						Factor* factor = obs_node->factors[this->inputs[i_index].second.first];

						factor->link();
					}

					obs_node->is_used = true;
				}
				break;
			}
		}

		this->is_used = true;
	}
}
