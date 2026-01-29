/**
 * - only update/split leaves
 *   - otherwise, too easy to destroy progress/naunce
 *     - especially with noise
 */

#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class AbstractDecisionTreeNode;
class EvalNode;

class DecisionTree {
public:
	int node_counter;
	std::map<int, AbstractDecisionTreeNode*> nodes;

	AbstractDecisionTreeNode* root;

	std::vector<double> improvement_history;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;
	int history_index;

	DecisionTree();
	~DecisionTree();

	double activate(std::vector<double>& obs);
	void backprop(std::vector<double>& obs,
				  double target_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(DecisionTree* original);

	void init_helper();
	void update_helper(EvalNode* node);
	void measure_helper();
};

#endif /* DECISION_TREE_H */