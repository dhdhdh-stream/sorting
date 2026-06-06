#include "solution.h"

#include <set>

#include "action_node.h"
#include "branch_network.h"
#include "branch_node.h"
#include "end_node.h"
#include "experiment.h"
#include "globals.h"
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

void Solution::random_exit_activate(AbstractNode* starting_node,
									vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				possible_exits.push_back(curr_node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_END:
			possible_exits.push_back(curr_node);
			curr_node = NULL;
			break;
		}
	}
}

void Solution::pad_new_state(int num_add) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)it->second;
				if (start_node->experiment != NULL) {
					Experiment* experiment = (Experiment*)start_node->experiment;
					experiment->pad_new_state(num_add);
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				if (action_node->experiment != NULL) {
					Experiment* experiment = (Experiment*)action_node->experiment;
					experiment->pad_new_state(num_add);
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;

				branch_node->original_network->add_inputs(num_add);
				branch_node->branch_network->add_inputs(num_add);

				if (branch_node->original_experiment != NULL) {
					Experiment* experiment = (Experiment*)branch_node->original_experiment;
					experiment->pad_new_state(num_add);
				}
				if (branch_node->branch_experiment != NULL) {
					Experiment* experiment = (Experiment*)branch_node->branch_experiment;
					experiment->pad_new_state(num_add);
				}
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

	output_file << this->ramp_last_scores.size() << endl;
	for (list<double>::iterator it = this->ramp_last_scores.begin();
			it != this->ramp_last_scores.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
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

	string num_ramp_last_scores_line;
	getline(input_file, num_ramp_last_scores_line);
	int num_ramp_last_scores = stoi(num_ramp_last_scores_line);
	for (int e_index = 0; e_index < num_ramp_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->ramp_last_scores.push_back(stod(score_line));
	}

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));

		string change_line;
		getline(input_file, change_line);
		this->change_history.push_back(change_line);
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
