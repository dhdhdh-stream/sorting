#include "exit_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ExitNode::ExitNode() {
	this->type = NODE_TYPE_EXIT;
}

ExitNode::~ExitNode() {
	// do nothing
}

void ExitNode::activate(vector<ContextLayer>& context,
						int& exit_depth,
						RunHelper& run_helper) {
	if (this->throw_id != -1) {
		run_helper.throw_id = this->throw_id;
	} else {
		vector<int> context_match_indexes;
		context_match_indexes.push_back((int)context.size()-1);
		int c_index = (int)this->scope_context.size()-2;
		int l_index = (int)context.size()-2;
		while (true) {
			if (c_index < 0) {
				break;
			}

			if (this->scope_context[c_index] == context[l_index].scope
					&& this->node_context[c_index] == context[l_index].node) {
				context_match_indexes.insert(context_match_indexes.begin(), l_index);
				c_index--;
			}
			l_index--;
		}

		int exit_index = context_match_indexes[context_match_indexes.size()-1 - this->exit_depth];
		exit_depth = context_match_indexes.back() - exit_index;
	}
}

void ExitNode::save(ofstream& output_file) {
	output_file << this->scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		output_file << this->scope_context_ids[c_index] << endl;
		output_file << this->node_context_ids[c_index] << endl;
	}
	output_file << this->exit_depth << endl;

	output_file << this->throw_id << endl;
}

void ExitNode::load(ifstream& input_file) {
	string branch_scope_context_size_line;
	getline(input_file, branch_scope_context_size_line);
	int branch_scope_context_size = stoi(branch_scope_context_size_line);
	for (int c_index = 0; c_index < branch_scope_context_size; c_index++) {
		string scope_context_id_line;
		getline(input_file, scope_context_id_line);
		this->scope_context_ids.push_back(stoi(scope_context_id_line));

		string node_context_id_line;
		getline(input_file, node_context_id_line);
		this->node_context_ids.push_back(stoi(node_context_id_line));
	}

	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string throw_id_line;
	getline(input_file, throw_id_line);
	this->throw_id = stoi(throw_id_line);
}

void ExitNode::link() {
	for (int c_index = 0; c_index < (int)this->scope_context_ids.size(); c_index++) {
		Scope* scope = solution->scopes[this->scope_context_ids[c_index]];
		this->scope_context.push_back(scope);
		this->node_context.push_back(scope->nodes[this->node_context_ids[c_index]]);
	}
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->throw_id << endl;
}
