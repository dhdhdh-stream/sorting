#include "logic_experiment.h"

#include "constants.h"
#include "eval_node.h"
#include "logic_helpers.h"
#include "logic_tree.h"
#include "network.h"
#include "split_node.h"

using namespace std;

LogicExperiment::LogicExperiment(AbstractLogicNode* node_context) {
	this->node_context = node_context;

	this->eval_network = NULL;

	this->state = LOGIC_EXPERIMENT_STATE_GATHER;
}

LogicExperiment::~LogicExperiment() {
	if (this->eval_network != NULL) {
		delete this->eval_network;
	}
}

void LogicExperiment::add(LogicTree* logic_tree) {
	double branch_percent = (double)this->count / (double)MEASURE_ITERS;

	EvalNode* new_eval_node = new EvalNode();
	new_eval_node->id = logic_tree->node_counter;
	logic_tree->node_counter++;
	logic_tree->nodes[new_eval_node->id] = new_eval_node;

	new_eval_node->network = this->eval_network;
	this->eval_network = NULL;

	new_eval_node->weight = this->node_context->weight * branch_percent;

	SplitNode* new_split_node = new SplitNode();
	new_split_node->id = logic_tree->node_counter;
	logic_tree->node_counter++;
	logic_tree->nodes[new_split_node->id] = new_split_node;

	new_split_node->obs_index = this->obs_index;
	new_split_node->split_type = this->split_type;
	new_split_node->split_target = this->split_target;

	new_split_node->original_node_id = this->node_context->id;
	new_split_node->original_node = this->node_context;
	new_split_node->branch_node_id = new_eval_node->id;
	new_split_node->branch_node = new_eval_node;

	new_split_node->weight = this->node_context->weight;

	SplitNode* parent = NULL;
	bool is_branch;
	find_parent(this->node_context,
				logic_tree,
				parent,
				is_branch);
	if (parent == NULL) {
		logic_tree->root = new_split_node;
	} else {
		if (is_branch) {
			parent->branch_node_id = new_split_node->id;
			parent->branch_node = new_split_node;
		} else {
			parent->original_node_id = new_split_node->id;
			parent->original_node = new_split_node;
		}
	}

	update_weight_helper(this->node_context,
						 1.0 - branch_percent);
}
