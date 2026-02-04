#include "split_node.h"

#include "decision_tree.h"
#include "network.h"

using namespace std;

bool is_match_helper(vector<double>& obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range) {
	switch (split_type) {
	case SPLIT_TYPE_GREATER:
		if (obs[obs_index] > split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_GREATER_EQUAL:
		if (obs[obs_index] >= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_LESSER:
		if (obs[obs_index] < split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_LESSER_EQUAL:
		if (obs[obs_index] <= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHIN:
		if (abs(obs[obs_index] - split_target) < split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHIN_EQUAL:
		if (abs(obs[obs_index] - split_target) <= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHOUT:
		if (abs(obs[obs_index] - split_target) > split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHOUT_EQUAL:
		if (abs(obs[obs_index] - split_target) >= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER:
		if (obs[obs_index] - obs[rel_obs_index] > split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER_EQUAL:
		if (obs[obs_index] - obs[rel_obs_index] >= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) < split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN_EQUAL:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) <= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) > split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT_EQUAL:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) >= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	}

	return false;	// unreachable
}

SplitNode::SplitNode() {
	this->type = DECISION_TREE_NODE_TYPE_SPLIT;
}

void SplitNode::save(ofstream& output_file) {
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;
}

void SplitNode::load(ifstream& input_file) {
	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string rel_obs_index_line;
	getline(input_file, rel_obs_index_line);
	this->rel_obs_index = stoi(rel_obs_index_line);

	string split_type_line;
	getline(input_file, split_type_line);
	this->split_type = stoi(split_type_line);

	string split_target_line;
	getline(input_file, split_target_line);
	this->split_target = stod(split_target_line);

	string split_range_line;
	getline(input_file, split_range_line);
	this->split_range = stod(split_range_line);

	string original_node_id_line;
	getline(input_file, original_node_id_line);
	this->original_node_id = stoi(original_node_id_line);

	string branch_node_id_line;
	getline(input_file, branch_node_id_line);
	this->branch_node_id = stoi(branch_node_id_line);
}

void SplitNode::link(DecisionTree* decision_tree) {
	this->original_node = decision_tree->nodes[this->original_node_id];
	this->branch_node = decision_tree->nodes[this->branch_node_id];
}

void SplitNode::copy_from(SplitNode* original) {
	this->obs_index = original->obs_index;
	this->rel_obs_index = original->rel_obs_index;
	this->split_type = original->split_type;
	this->split_target = original->split_target;
	this->split_range = original->split_range;

	this->original_node_id = original->original_node_id;
	this->branch_node_id = original->branch_node_id;
}
