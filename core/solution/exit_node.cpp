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

void ExitNode::random_activate(vector<Scope*>& scope_context,
							   vector<AbstractNode*>& node_context,
							   int& exit_depth,
							   int& random_throw_id) {
	if (this->throw_id != -1) {
		random_throw_id = this->throw_id;
	} else {
		vector<int> context_match_indexes;
		context_match_indexes.push_back((int)scope_context.size()-1);
		int c_index = (int)this->scope_context.size()-2;
		int l_index = (int)scope_context.size()-2;
		while (true) {
			if (c_index < 0) {
				break;
			}

			if (this->scope_context[c_index] == scope_context[l_index]
					&& this->node_context[c_index] == node_context[l_index]) {
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
	output_file << this->exit_depth << endl;

	output_file << this->throw_id << endl;
}

void ExitNode::load(ifstream& input_file) {
	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string throw_id_line;
	getline(input_file, throw_id_line);
	this->throw_id = stoi(throw_id_line);
}

void ExitNode::link() {
	// do nothing
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->throw_id << endl;
}
