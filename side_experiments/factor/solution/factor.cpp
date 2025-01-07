#include "factor.h"

using namespace std;

Factor::Factor() {
	// do nothing
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

	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope
					&& this->input_node_context_ids[i_index][l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
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

	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
		}
	}
}

void BranchNode::save(ofstream& output_file) {
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

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
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
			scope_ids.push_back(parent_solution->scopes[stoi(scope_id_line)]);

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

void Factor::link(Solution* parent_solution) {
	if (this->input_scope_contexts.size() == 0) {
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			Scope* scope = this->inputs[i_index].first.first.back();
			AbstractNode* node = scope->nodes[this->inputs[i_index].first.second.back()];
			switch (node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;

					bool is_existing = false;
					for (int ii_index = 0; ii_index < (int)branch_node->input_scope_contexts.size(); ii_index++) {
						if (branch_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
								&& branch_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						branch_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
						branch_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;

					if (this->inputs[i_index].second.first == -1) {
						bool is_existing = false;
						for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_contexts.size(); ii_index++) {
							if (obs_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
									&& obs_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second
									&& obs_node->input_obs_indexes[ii_index] == this->inputs[i_index].second.second) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							obs_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
							obs_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
							obs_node->input_obs_indexes.push_back(this->inputs[i_index].second.second);
						}
					} else {
						Factor* factor = obs_node->factors[this->inputs[i_index].second.first];

						factor->link(parent_solution);

						bool is_existing = false;
						for (int ii_index = 0; ii_index < (int)factor->input_scope_contexts.size(); ii_index++) {
							if (factor->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
									&& factor->input_node_context_ids[ii_index] == this->inputs[i_index].first.second) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							factor->input_scope_contexts.push_back(this->inputs[i_index].first.first);
							factor->input_node_context_ids.push_back(this->inputs[i_index].first.second);
						}
					}
				}
				break;
			}
		}
	}
}
