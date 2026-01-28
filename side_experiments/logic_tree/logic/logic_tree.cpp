#include "logic_tree.h"

#include <iostream>
#include <set>

#include "eval_node.h"
#include "split_node.h"

using namespace std;

LogicTree::~LogicTree() {
	for (map<int, AbstractLogicNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}
}

void LogicTree::clean() {
	set<int> accessible_node_indexes;
	accessible_node_indexes.insert(this->root->id);
	for (map<int, AbstractLogicNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case LOGIC_NODE_TYPE_SPLIT:
			{
				SplitNode* split_node = (SplitNode*)it->second;
				accessible_node_indexes.insert(split_node->original_node_id);
				accessible_node_indexes.insert(split_node->branch_node_id);
			}
			break;
		}
	}

	map<int, AbstractLogicNode*>::iterator it = this->nodes.begin();
	while (it != this->nodes.end()) {
		if (accessible_node_indexes.find(it->first) == accessible_node_indexes.end()) {
			it = this->nodes.erase(it);
		} else {
			it++;
		}
	}
}

void LogicTree::save(string path,
					 string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractLogicNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->root->id << endl;

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void LogicTree::load(string path,
					 string name) {
	ifstream input_file;
	input_file.open(path + name);

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
		case LOGIC_NODE_TYPE_SPLIT:
			{
				SplitNode* split_node = new SplitNode();
				split_node->id = id;
				split_node->load(input_file);
				this->nodes[split_node->id] = split_node;
			}
			break;
		case LOGIC_NODE_TYPE_EVAL:
			{
				EvalNode* eval_node = new EvalNode();
				eval_node->id = id;
				eval_node->load(input_file);
				this->nodes[eval_node->id] = eval_node;
			}
			break;
		}
	}

	for (map<int, AbstractLogicNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case LOGIC_NODE_TYPE_SPLIT:
			{
				SplitNode* split_node = (SplitNode*)it->second;
				split_node->link(this);
			}
			break;
		}
	}

	string root_id_line;
	getline(input_file, root_id_line);
	this->root = this->nodes[stoi(root_id_line)];

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));
	}

	input_file.close();
}
