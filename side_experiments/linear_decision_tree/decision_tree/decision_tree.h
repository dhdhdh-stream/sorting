/**
 * - only update/split leaves
 *   - otherwise, too easy to destroy progress/naunce
 *     - especially with noise
 */

/**
 * TODO: only gather 1 sample per run(?)
 */

#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class AbstractDecisionTreeNode;
class EvalNode;

#if defined(MDEBUG) && MDEBUG
const int DT_NUM_TRAIN_SAMPLES = 40;
const int DT_NUM_TEST_SAMPLES = 10;
#else
// const int DT_NUM_TRAIN_SAMPLES = 800;
const int DT_NUM_TRAIN_SAMPLES = 4000;
// const int DT_NUM_TEST_SAMPLES = 200;
const int DT_NUM_TEST_SAMPLES = 1000;
#endif /* MDEBUG */
const int DT_NUM_TOTAL_SAMPLES = DT_NUM_TRAIN_SAMPLES + DT_NUM_TEST_SAMPLES;

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