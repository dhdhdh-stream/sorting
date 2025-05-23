#include "factor.h"

#include "branch_node.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

Factor::Factor() {
	this->is_used = false;
}

Factor::Factor(Factor* original,
			   Solution* parent_solution) {
	this->is_used = original->is_used;

	this->inputs = original->inputs;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			this->inputs[i_index].scope_context[l_index] =
				parent_solution->scopes[this->inputs[i_index].scope_context[l_index]->id];
		}
	}
	this->network = new Network(original->network);
}

Factor::~Factor() {
	delete this->network;
}

void Factor::clean_inputs(Scope* scope,
						  int node_id) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope
					&& this->inputs[i_index].node_context[l_index] == node_id) {
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
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope) {
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

void Factor::replace_factor(Scope* scope,
							int original_node_id,
							int original_factor_index,
							int new_node_id,
							int new_factor_index) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id
				&& this->inputs[i_index].factor_index == original_factor_index) {
			this->inputs[i_index].node_context.back() = new_node_id;
			this->inputs[i_index].factor_index = new_factor_index;
		}
	}
}

void Factor::replace_obs_node(Scope* scope,
							  int original_node_id,
							  int new_node_id) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id) {
			this->inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void Factor::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		this->inputs[i_index].save(output_file);
	}

	this->network->save(output_file);
}

void Factor::load(ifstream& input_file,
				  Solution* parent_solution) {
	this->is_used = false;

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->inputs.push_back(Input(input_file,
									 parent_solution));
	}

	this->network = new Network(input_file);
}

void Factor::link(Solution* parent_solution) {
	if (!this->is_used) {
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			Scope* scope = this->inputs[i_index].scope_context.back();
			AbstractNode* node = scope->nodes[this->inputs[i_index].node_context.back()];
			switch (node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;
					branch_node->is_used = true;
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;

					if (this->inputs[i_index].factor_index != -1) {
						Factor* factor = obs_node->factors[this->inputs[i_index].factor_index];

						factor->link(parent_solution);
					}

					obs_node->is_used = true;
				}
				break;
			}
		}
	}
}
