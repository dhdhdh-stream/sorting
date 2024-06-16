#include "info_scope.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"

using namespace std;

InfoScope::InfoScope() {
	this->experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

InfoScope::~InfoScope() {
	delete this->subscope;

	delete this->network;
}

#if defined(MDEBUG) && MDEBUG
void InfoScope::clear_verify() {
	this->experiment = NULL;

	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("info scope remaining verify");
	}
}
#endif /* MDEBUG */

void InfoScope::save(ofstream& output_file) {
	this->subscope->save(output_file);

	output_file << this->input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		output_file << this->input_node_contexts[i_index]->id << endl;
		output_file << this->input_obs_indexes[i_index] << endl;
	}
	this->network->save(output_file);
}

void InfoScope::load(ifstream& input_file) {
	this->subscope = new Scope();
	this->subscope->load(input_file);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string node_context_id_line;
		getline(input_file, node_context_id_line);
		this->input_node_contexts.push_back(this->subscope->nodes[stoi(node_context_id_line)]);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}
	this->network = new Network(input_file);
}

void InfoScope::link(Solution* parent_solution) {
	this->subscope->link(parent_solution);
}

void InfoScope::copy_from(InfoScope* original,
						  Solution* parent_solution) {
	this->subscope = new Scope();
	this->subscope->copy_from(original->subscope,
							  parent_solution);

	for (int i_index = 0; i_index < (int)original->input_node_contexts.size(); i_index++) {
		this->input_node_contexts.push_back(this->subscope->nodes[
			original->input_node_contexts[i_index]->id]);
	}
	this->input_obs_indexes = original->input_obs_indexes;
	this->network = new Network(original->network);
}
