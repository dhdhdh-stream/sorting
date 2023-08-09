#include "exit_node.h"

#include <iostream>

#include "constants.h"
#include "exit_network.h"
#include "layer.h"
#include "scope.h"

using namespace std;

ExitNode::ExitNode(Scope* parent,
				   int id,
				   int exit_depth,
				   int exit_node_id,
				   vector<int> target_indexes,
				   vector<ExitNetwork*> networks) {
	this->type = NODE_TYPE_EXIT;

	this->parent = parent;
	this->id = id;

	this->exit_depth = exit_depth;
	this->exit_node_id = exit_node_id;
	this->target_indexes = target_indexes;
	this->networks = networks;
}

ExitNode::ExitNode(ExitNode* original,
				   Scope* parent,
				   int id,
				   int exit_node_id) {
	this->type = NODE_TYPE_EXIT;

	this->parent = parent;
	this->id = id;

	this->exit_depth = 0;
	this->exit_node_id = exit_node_id;
	this->target_indexes = original->target_indexes;
	for (int e_index = 0; e_index < (int)original->networks.size(); e_index++) {
		this->networks.push_back(new ExitNetwork(original->networks[e_index]));
	}
}

ExitNode::ExitNode(ifstream& input_file,
				   Scope* parent,
				   int id) {
	this->type = NODE_TYPE_EXIT;

	this->parent = parent;
	this->id = id;

	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string exit_node_id_line;
	getline(input_file, exit_node_id_line);
	this->exit_node_id = stoi(exit_node_id_line);

	string networks_size_line;
	getline(input_file, networks_size_line);
	int networks_size = stoi(networks_size_line);
	for (int s_index = 0; s_index < networks_size; s_index++) {
		string target_index_line;
		getline(input_file, target_index_line);
		this->target_indexes.push_back(stoi(target_index_line));

		ifstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_exit_" + to_string(s_index) + ".txt");
		this->networks.push_back(new ExitNetwork(network_save_file));
		network_save_file.close();
	}
}

ExitNode::~ExitNode() {
	for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
		delete this->networks[s_index];
	}
}

void ExitNode::activate(vector<ForwardContextLayer>& context,
						RunHelper& run_helper,
						ExitNodeHistory* history) {
	history->state_vals_snapshot = vector<vector<double>>(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
		history->state_vals_snapshot[l_index] = *(context[
			context.size() - (this->exit_depth+1) + l_index].state_vals);
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		history->network_histories = vector<ExitNetworkHistory*>(this->networks.size(), NULL);
		for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
			if (context[context.size() - (this->exit_depth+1)].states_initialized[s_index]) {
				ExitNetwork* network = this->networks[s_index];
				ExitNetworkHistory* network_history = new ExitNetworkHistory(network);
				network->activate(history->state_vals_snapshot,
								  network_history);
				history->network_histories[s_index] = network_history;
				context[context.size() - (this->exit_depth+1)].state_vals->at(this->target_indexes[s_index]) += network->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
			if (context[context.size() - (this->exit_depth+1)].states_initialized[s_index]) {
				ExitNetwork* network = this->networks[s_index];
				network->activate(history->state_vals_snapshot);
				context[context.size() - (this->exit_depth+1)].state_vals->at(this->target_indexes[s_index]) += network->output->acti_vals[0];
			}
		}
	}
}

void ExitNode::backprop(vector<BackwardContextLayer>& context,
						RunHelper& run_helper,
						ExitNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (!run_helper.backprop_is_pre_experiment) {
			vector<vector<double>*> state_errors(this->exit_depth+1);
			for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
				state_errors[l_index] = context[
					context.size() - (this->exit_depth+1) + l_index].state_errors;
			}

			vector<double> outer_state_errors_snapshot = *(context[context.size() - (this->exit_depth+1)].state_errors);

			for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
				if (history->network_histories[s_index] != NULL) {
					ExitNetwork* network = history->network_histories[s_index]->network;
					network->backprop_errors_with_no_weight_change(
						outer_state_errors_snapshot[this->target_indexes[s_index]],
						state_errors,
						history->state_vals_snapshot,
						history->network_histories[s_index]);
				}
			}
		}
	}
}

void ExitNode::save(ofstream& output_file) {
	output_file << this->exit_depth << endl;
	output_file << this->exit_node_id << endl;

	output_file << this->networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->networks.size(); s_index++) {
		output_file << this->target_indexes[s_index] << endl;

		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_exit_" + to_string(s_index) + ".txt");
		this->networks[s_index]->save(network_save_file);
		network_save_file.close();
	}
}

void ExitNode::save_for_display(ofstream& output_file) {

}

ExitNodeHistory::ExitNodeHistory(ExitNode* node) {
	this->node = node;
}

ExitNodeHistory::~ExitNodeHistory() {
	for (int s_index = 0; s_index < (int)this->network_histories.size(); s_index++) {
		if (this->network_histories[s_index] != NULL) {
			delete this->network_histories[s_index];
		}
	}
}
