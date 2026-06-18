#include "solution.h"

#include <set>

#include "action_node.h"
#include "branch_node.h"
#include "end_node.h"
#include "globals.h"
#include "network.h"
#include "start_node.h"

using namespace std;

Solution::Solution() {
	this->score_index = 0;
}

Solution::~Solution() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}
}

void Solution::pad_new_state(int num_add) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)it->second;

				start_node->state_history.clear();
				start_node->history_index = 0;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;

				action_node->state_history.clear();
				action_node->history_index = 0;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;

				branch_node->original_network->add_inputs(num_add);
				branch_node->branch_network->add_inputs(num_add);

				branch_node->original_state_history.clear();
				branch_node->original_target_val_history.clear();
				branch_node->original_history_index = 0;
				branch_node->branch_state_history.clear();
				branch_node->branch_target_val_history.clear();
				branch_node->branch_history_index = 0;
			}
			break;
		}
	}
}

void Solution::save(ofstream& output_file,
					Wrapper* wrapper) {
	/**
	 * TODO: swap endl to '\n' to avoid flushing
	 */
	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file,
						 wrapper);
	}

	output_file << this->train_new_last_scores.size() << endl;
	for (list<double>::iterator it = this->train_new_last_scores.begin();
			it != this->train_new_last_scores.end(); it++) {
		output_file << *it << endl;
	}
}

void Solution::load(ifstream& input_file,
					Wrapper* wrapper) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string node_counter_line;
	getline(input_file, node_counter_line);
	this->node_counter = stoi(node_counter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string id_line;
		getline(input_file, id_line);
		int id = stoi(id_line);

		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		switch (type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = new StartNode();
				start_node->id = id;
				start_node->load(input_file,
								 wrapper);
				this->nodes[start_node->id] = start_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = new ActionNode();
				action_node->id = id;
				action_node->load(input_file);
				this->nodes[action_node->id] = action_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = new BranchNode();
				branch_node->id = id;
				branch_node->load(input_file,
								  wrapper);
				this->nodes[branch_node->id] = branch_node;
			}
			break;
		case NODE_TYPE_END:
			{
				EndNode* end_node = new EndNode();
				end_node->id = id;
				end_node->load(input_file,
							   wrapper);
				this->nodes[end_node->id] = end_node;
			}
			break;
		}
	}

	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(wrapper);
	}

	string num_train_new_last_scores_line;
	getline(input_file, num_train_new_last_scores_line);
	int num_train_new_last_scores = stoi(num_train_new_last_scores_line);
	for (int e_index = 0; e_index < num_train_new_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->train_new_last_scores.push_back(stod(score_line));
	}
}

void Solution::link(Wrapper* wrapper) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(wrapper);
	}
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save_for_display(output_file);
	}
}
