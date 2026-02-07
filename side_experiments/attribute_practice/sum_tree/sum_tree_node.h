#ifndef SUM_TREE_NODE_H
#define SUM_TREE_NODE_H

#include <fstream>
#include <vector>

class SumTree;

const int SUM_TREE_NODE_MAX_NUM_INPUTS = 10;

class SumTreeNode {
public:
	int id;

	double constant;
	std::vector<int> input_indexes;
	std::vector<double> input_weights;
	double previous_weight;

	bool has_split;
	int obs_index;
	int rel_obs_index;
	int split_type;
	double split_target;
	double split_range;

	int original_node_id;
	SumTreeNode* original_node;
	int branch_node_id;
	SumTreeNode* branch_node;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> previous_val_histories;
	std::vector<double> target_val_histories;

	double activate(std::vector<double>& obs,
					double previous_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(SumTree* decision_tree);

	void copy_from(SumTreeNode* original);
};

#endif /* SUM_TREE_NODE_H */