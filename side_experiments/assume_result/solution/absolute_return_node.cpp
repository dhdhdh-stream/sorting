#include "absolute_return_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"

using namespace std;

AbsoluteReturnNode::AbsoluteReturnNode() {
	this->type = NODE_TYPE_ABSOLUTE_RETURN;

	this->average_instances_per_run = 0.0;
}

AbsoluteReturnNode::AbsoluteReturnNode(AbsoluteReturnNode* original) {
	this->type = NODE_TYPE_ABSOLUTE_RETURN;

	this->location = problem_type->deep_copy_location(original->location);

	this->next_node_id = original->next_node_id;

	this->average_instances_per_run = 0.0;
}

AbsoluteReturnNode::~AbsoluteReturnNode() {
	delete this->location;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void AbsoluteReturnNode::save(ofstream& output_file) {
	problem_type->save_location(this->location,
								output_file);

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void AbsoluteReturnNode::load(ifstream& input_file) {
	this->location = problem_type->load_location(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void AbsoluteReturnNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void AbsoluteReturnNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}
