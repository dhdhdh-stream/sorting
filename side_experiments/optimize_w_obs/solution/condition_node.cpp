#include "condition_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

ConditionNode::ConditionNode() {
	this->type = NODE_TYPE_CONDITION;

	this->average_instances_per_run = 0.0;
}

ConditionNode::ConditionNode(ConditionNode* original) {
	this->type = NODE_TYPE_CONDITION;

	this->conditions = original->conditions;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->average_instances_per_run = 0.0;
}

ConditionNode::~ConditionNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ConditionNode::save(ofstream& output_file) {
	output_file << this->conditions.size() << endl;
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		output_file << this->conditions[c_index].first.first.size() << endl;
		for (int l_index = 0; l_index < (int)this->conditions[c_index].first.first.size(); l_index++) {
			output_file << this->conditions[c_index].first.first[l_index] << endl;
			output_file << this->conditions[c_index].first.second[l_index] << endl;
		}

		output_file << this->conditions[c_index].second << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void ConditionNode::load(ifstream& input_file) {
	string num_conditions_line;
	getline(input_file, num_conditions_line);
	int num_conditions = stoi(num_conditions_line);
	for (int c_index = 0; c_index < num_conditions; c_index++) {
		vector<int> scope_ids;
		vector<int> node_ids;

		string num_layers_line;
		getline(input_file, num_layers_line);
		int num_layers = stoi(num_layers_line);
		for (int l_index = 0; l_index < num_layers; l_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			scope_ids.push_back(stoi(scope_id_line));

			string node_id_line;
			getline(input_file, node_id_line);
			node_ids.push_back(stoi(node_id_line));
		}

		string is_branch_line;
		getline(input_file, is_branch_line);
		bool is_branch = stoi(is_branch_line);

		this->conditions.push_back({{scope_ids, node_ids}, is_branch});
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void ConditionNode::link(Solution* parent_solution) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void ConditionNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

ConditionNodeHistory::ConditionNodeHistory(ConditionNode* node) {
	this->node = node;
}

ConditionNodeHistory::ConditionNodeHistory(ConditionNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->is_branch = original->is_branch;
}
