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
	if (this->subscope != NULL) {
		delete this->subscope;
	}

	if (this->positive_network != NULL) {
		delete this->positive_network;
	}

	if (this->negative_network != NULL) {
		delete this->negative_network;
	}
}

#if defined(MDEBUG) && MDEBUG
void InfoScope::clear_verify() {
	this->experiment = NULL;

	this->verify_key = NULL;
	if (this->verify_negative_scores.size() > 0
			|| this->verify_positive_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("info scope remaining verify");
	}
}
#endif /* MDEBUG */

void InfoScope::save(ofstream& output_file) {
	output_file << this->state << endl;
	if (this->state == INFO_SCOPE_STATE_NA) {
		this->subscope->save(output_file);

		output_file << this->positive_input_node_contexts.size() << endl;
		for (int i_index = 0; i_index < (int)this->positive_input_node_contexts.size(); i_index++) {
			output_file << this->positive_input_node_contexts[i_index]->id << endl;
			output_file << this->positive_input_obs_indexes[i_index] << endl;
		}
		this->positive_network->save(output_file);

		output_file << this->negative_input_node_contexts.size() << endl;
		for (int i_index = 0; i_index < (int)this->negative_input_node_contexts.size(); i_index++) {
			output_file << this->negative_input_node_contexts[i_index]->id << endl;
			output_file << this->negative_input_obs_indexes[i_index] << endl;
		}
		this->negative_network->save(output_file);
	}
}

void InfoScope::load(ifstream& input_file) {
	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	if (this->state == INFO_SCOPE_STATE_NA) {
		this->subscope = new Scope();
		this->subscope->load(input_file);

		string positive_num_inputs_line;
		getline(input_file, positive_num_inputs_line);
		int positive_num_inputs = stoi(positive_num_inputs_line);
		for (int i_index = 0; i_index < positive_num_inputs; i_index++) {
			string node_context_id_line;
			getline(input_file, node_context_id_line);
			this->positive_input_node_contexts.push_back(this->subscope->nodes[stoi(node_context_id_line)]);

			string obs_index_line;
			getline(input_file, obs_index_line);
			this->positive_input_obs_indexes.push_back(stoi(obs_index_line));
		}
		this->positive_network = new Network(input_file);

		string negative_num_inputs_line;
		getline(input_file, negative_num_inputs_line);
		int negative_num_inputs = stoi(negative_num_inputs_line);
		for (int i_index = 0; i_index < negative_num_inputs; i_index++) {
			string node_context_id_line;
			getline(input_file, node_context_id_line);
			this->negative_input_node_contexts.push_back(this->subscope->nodes[stoi(node_context_id_line)]);

			string obs_index_line;
			getline(input_file, obs_index_line);
			this->negative_input_obs_indexes.push_back(stoi(obs_index_line));
		}
		this->negative_network = new Network(input_file);
	} else {
		this->subscope = NULL;
		this->positive_network = NULL;
		this->negative_network = NULL;
	}
}

void InfoScope::link(Solution* parent_solution) {
	if (this->subscope != NULL) {
		this->subscope->link(parent_solution);
	}
}

void InfoScope::copy_from(InfoScope* original,
						  Solution* parent_solution) {
	this->state = original->state;
	if (this->state == INFO_SCOPE_STATE_NA) {
		this->subscope = new Scope();
		this->subscope->copy_from(original->subscope,
								  parent_solution);

		for (int i_index = 0; i_index < (int)original->positive_input_node_contexts.size(); i_index++) {
			this->positive_input_node_contexts.push_back(this->subscope->nodes[
				original->positive_input_node_contexts[i_index]->id]);
		}
		this->positive_input_obs_indexes = original->positive_input_obs_indexes;
		this->positive_network = new Network(original->positive_network);

		for (int i_index = 0; i_index < (int)original->negative_input_node_contexts.size(); i_index++) {
			this->negative_input_node_contexts.push_back(this->subscope->nodes[
				original->negative_input_node_contexts[i_index]->id]);
		}
		this->negative_input_obs_indexes = original->negative_input_obs_indexes;
		this->negative_network = new Network(original->negative_network);
	} else {
		this->subscope = NULL;
		this->positive_network = NULL;
		this->negative_network = NULL;
	}
}
