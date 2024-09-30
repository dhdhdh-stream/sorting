#include "markov_node.h"

#include "abstract_experiment.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

MarkovNode::MarkovNode() {
	this->type = NODE_TYPE_MARKOV;

	this->average_instances_per_run = 0.0;
}

MarkovNode::MarkovNode(MarkovNode* original,
					   Solution* parent_solution) {
	this->type = NODE_TYPE_MARKOV;

	this->step_types = original->step_types;
	this->actions = original->actions;
	this->scopes = original->scopes;
	for (int o_index = 0; o_index < (int)this->scopes.size(); o_index++) {
		for (int s_index = 0; s_index < (int)this->scopes[o_index].size(); s_index++) {
			if (this->scopes[o_index][s_index] != NULL) {
				this->scopes[o_index][s_index] = parent_solution->scopes[this->scopes[o_index][s_index]->id];
			}
		}
	}
	this->networks = vector<Network*>(original->networks.size());
	for (int n_index = 0; n_index < (int)original->networks.size(); n_index++) {
		this->networks[n_index] = new Network(original->networks[n_index]);
	}
	this->halt_network = new Network(original->halt_network);

	this->next_node_id = original->next_node_id;

	this->average_instances_per_run = 0.0;
}

MarkovNode::~MarkovNode() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}
	delete this->halt_network;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void MarkovNode::save(ofstream& output_file) {
	output_file << this->step_types.size() << endl;

	for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
		output_file << this->step_types[o_index].size() << endl;
		for (int s_index = 0; s_index < (int)this->step_types[o_index].size(); s_index++) {
			output_file << this->step_types[o_index][s_index] << endl;
			switch (this->step_types[o_index][s_index]) {
			case MARKOV_STEP_TYPE_ACTION:
				this->actions[o_index][s_index].save(output_file);
				break;
			case MARKOV_STEP_TYPE_SCOPE:
				output_file << this->scopes[o_index][s_index]->id << endl;
				break;
			}
			
		}
	}

	for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
		this->networks[o_index]->save(output_file);
	}
	this->halt_network->save(output_file);

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void MarkovNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string num_options_line;
	getline(input_file, num_options_line);
	int num_options = stoi(num_options_line);

	this->step_types = vector<vector<int>>(num_options);
	this->actions = vector<vector<Action>>(num_options);
	this->scopes = vector<vector<Scope*>>(num_options);
	for (int o_index = 0; o_index < num_options; o_index++) {
		string num_steps_line;
		getline(input_file, num_steps_line);
		int num_steps = stoi(num_steps_line);

		this->step_types[o_index] = vector<int>(num_steps);
		this->actions[o_index] = vector<Action>(num_steps);
		this->scopes[o_index] = vector<Scope*>(num_steps, NULL);

		for (int s_index = 0; s_index < num_steps; s_index++) {
			string step_type_line;
			getline(input_file, step_type_line);
			this->step_types[o_index][s_index] = stoi(step_type_line);
			switch (this->step_types[o_index][s_index]) {
			case MARKOV_STEP_TYPE_ACTION:
				this->actions[o_index][s_index] = Action(input_file);
				break;
			case MARKOV_STEP_TYPE_SCOPE:
				{
					string scope_id_line;
					getline(input_file, scope_id_line);
					this->scopes[o_index][s_index] = parent_solution->scopes[stoi(scope_id_line)];
				}
				break;
			}
		}
	}

	this->networks = vector<Network*>(num_options);
	for (int o_index = 0; o_index < num_options; o_index++) {
		this->networks[o_index] = new Network(input_file);
	}
	this->halt_network = new Network(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void MarkovNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void MarkovNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}
