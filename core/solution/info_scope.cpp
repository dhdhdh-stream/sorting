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

		output_file << this->positive_average_score << endl;
		output_file << this->negative_average_score << endl;

		output_file << this->input_node_contexts.size() << endl;
		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
			output_file << this->input_node_contexts[i_index]->id << endl;
			output_file << this->input_obs_indexes[i_index] << endl;
		}

		output_file << this->linear_positive_input_indexes.size() << endl;
		for (int i_index = 0; i_index < (int)this->linear_positive_input_indexes.size(); i_index++) {
			output_file << this->linear_positive_input_indexes[i_index] << endl;
			output_file << this->linear_positive_weights[i_index] << endl;
		}

		output_file << this->linear_negative_input_indexes.size() << endl;
		for (int i_index = 0; i_index < (int)this->linear_negative_input_indexes.size(); i_index++) {
			output_file << this->linear_negative_input_indexes[i_index] << endl;
			output_file << this->linear_negative_weights[i_index] << endl;
		}

		output_file << this->positive_network_input_indexes.size() << endl;
		for (int i_index = 0; i_index < (int)this->positive_network_input_indexes.size(); i_index++) {
			output_file << this->positive_network_input_indexes[i_index].size() << endl;
			for (int v_index = 0; v_index < (int)this->positive_network_input_indexes[i_index].size(); v_index++) {
				output_file << this->positive_network_input_indexes[i_index][v_index] << endl;
			}
		}
		if (this->positive_network != NULL) {
			this->positive_network->save(output_file);
		}

		output_file << this->negative_network_input_indexes.size() << endl;
		for (int i_index = 0; i_index < (int)this->negative_network_input_indexes.size(); i_index++) {
			output_file << this->negative_network_input_indexes[i_index].size() << endl;
			for (int v_index = 0; v_index < (int)this->negative_network_input_indexes[i_index].size(); v_index++) {
				output_file << this->negative_network_input_indexes[i_index][v_index] << endl;
			}
		}
		if (this->negative_network != NULL) {
			this->negative_network->save(output_file);
		}
	}
}

void InfoScope::load(ifstream& input_file) {
	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	if (this->state == INFO_SCOPE_STATE_NA) {
		this->subscope = new Scope();
		this->subscope->load(input_file);

		string positive_average_score_line;
		getline(input_file, positive_average_score_line);
		this->positive_average_score = stod(positive_average_score_line);

		string negative_average_score_line;
		getline(input_file, negative_average_score_line);
		this->negative_average_score = stod(negative_average_score_line);

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

		string num_linear_positive_inputs_line;
		getline(input_file, num_linear_positive_inputs_line);
		int num_linear_positive_inputs = stoi(num_linear_positive_inputs_line);
		for (int i_index = 0; i_index < num_linear_positive_inputs; i_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			this->linear_positive_input_indexes.push_back(stoi(input_index_line));

			string weight_line;
			getline(input_file, weight_line);
			this->linear_positive_weights.push_back(stod(weight_line));
		}

		string num_linear_negative_inputs_line;
		getline(input_file, num_linear_negative_inputs_line);
		int num_linear_negative_inputs = stoi(num_linear_negative_inputs_line);
		for (int i_index = 0; i_index < num_linear_negative_inputs; i_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			this->linear_negative_input_indexes.push_back(stoi(input_index_line));

			string weight_line;
			getline(input_file, weight_line);
			this->linear_negative_weights.push_back(stod(weight_line));
		}

		string num_positive_network_inputs_line;
		getline(input_file, num_positive_network_inputs_line);
		int num_positive_network_inputs = stoi(num_positive_network_inputs_line);
		for (int i_index = 0; i_index < num_positive_network_inputs; i_index++) {
			string layer_size_line;
			getline(input_file, layer_size_line);
			int layer_size = stoi(layer_size_line);
			vector<int> v_input_indexes;
			for (int v_index = 0; v_index < layer_size; v_index++) {
				string input_index_line;
				getline(input_file, input_index_line);
				v_input_indexes.push_back(stoi(input_index_line));
			}
			this->positive_network_input_indexes.push_back(v_input_indexes);
		}
		if (this->positive_network_input_indexes.size() == 0) {
			this->positive_network = NULL;
		} else {
			this->positive_network = new Network(input_file);
		}

		string num_negative_network_inputs_line;
		getline(input_file, num_negative_network_inputs_line);
		int num_negative_network_inputs = stoi(num_negative_network_inputs_line);
		for (int i_index = 0; i_index < num_negative_network_inputs; i_index++) {
			string layer_size_line;
			getline(input_file, layer_size_line);
			int layer_size = stoi(layer_size_line);
			vector<int> v_input_indexes;
			for (int v_index = 0; v_index < layer_size; v_index++) {
				string input_index_line;
				getline(input_file, input_index_line);
				v_input_indexes.push_back(stoi(input_index_line));
			}
			this->negative_network_input_indexes.push_back(v_input_indexes);
		}
		if (this->negative_network_input_indexes.size() == 0) {
			this->negative_network = NULL;
		} else {
			this->negative_network = new Network(input_file);
		}
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

		this->positive_average_score = original->positive_average_score;
		this->negative_average_score = original->negative_average_score;

		for (int i_index = 0; i_index < (int)original->input_node_contexts.size(); i_index++) {
			this->input_node_contexts.push_back(this->subscope->nodes[
				original->input_node_contexts[i_index]->id]);
		}
		this->input_obs_indexes = original->input_obs_indexes;

		this->linear_positive_input_indexes = original->linear_positive_input_indexes;
		this->linear_positive_weights = original->linear_positive_weights;

		this->linear_negative_input_indexes = original->linear_negative_input_indexes;
		this->linear_negative_weights = original->linear_negative_weights;

		this->positive_network_input_indexes = original->positive_network_input_indexes;
		if (original->positive_network == NULL) {
			this->positive_network = NULL;
		} else {
			this->positive_network = new Network(original->positive_network);
		}

		this->negative_network_input_indexes = original->negative_network_input_indexes;
		if (original->negative_network == NULL) {
			this->negative_network = NULL;
		} else {
			this->negative_network = new Network(original->negative_network);
		}
	} else {
		this->subscope = NULL;
		this->positive_network = NULL;
		this->negative_network = NULL;
	}
}
