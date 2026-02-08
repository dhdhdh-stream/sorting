#ifndef DECISION_TREE_NODE_H
#define DECISION_TREE_NODE_H

#include <fstream>
#include <vector>

class DecisionTree;
class Network;

const int DT_NODE_MAX_NUM_INPUTS = 10;

class DecisionTreeNode {
public:
	int id;

	bool is_previous;
	std::vector<int> input_indexes;
	/**
	 * - num network inputs is 1 + input_indexes.size()
	 *   - with 1st input being previous val
	 */
	Network* network;

	bool has_split;
	int obs_index;
	int rel_obs_index;
	int split_type;
	double split_target;
	double split_range;

	int original_node_id;
	DecisionTreeNode* original_node;
	int branch_node_id;
	DecisionTreeNode* branch_node;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> previous_val_histories;
	std::vector<double> target_val_histories;

	DecisionTreeNode();
	~DecisionTreeNode();

	double activate(std::vector<double>& obs,
					double previous_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(DecisionTree* decision_tree);

	void copy_from(DecisionTreeNode* original);
};

#endif /* DECISION_TREE_NODE_H */