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

class DecisionTreeNode;

#if defined(MDEBUG) && MDEBUG
const int DT_NUM_TRAIN_SAMPLES = 40;
const int DT_NUM_TEST_SAMPLES = 10;
#else
const int DT_NUM_TRAIN_SAMPLES = 4000;
const int DT_NUM_TEST_SAMPLES = 1000;
#endif /* MDEBUG */
const int DT_NUM_TOTAL_SAMPLES = DT_NUM_TRAIN_SAMPLES + DT_NUM_TEST_SAMPLES;

class DecisionTree {
public:
	int node_counter;
	std::map<int, DecisionTreeNode*> nodes;

	DecisionTreeNode* root;

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
	void update_helper(DecisionTreeNode* node);
	void measure_helper();
};

const int SPLIT_TYPE_GREATER = 0;
const int SPLIT_TYPE_GREATER_EQUAL = 1;
const int SPLIT_TYPE_LESSER = 2;
const int SPLIT_TYPE_LESSER_EQUAL = 3;
const int SPLIT_TYPE_WITHIN = 4;
const int SPLIT_TYPE_WITHIN_EQUAL = 5;
const int SPLIT_TYPE_WITHOUT = 6;
const int SPLIT_TYPE_WITHOUT_EQUAL = 7;
const int SPLIT_TYPE_REL_GREATER = 8;
const int SPLIT_TYPE_REL_GREATER_EQUAL = 9;
const int SPLIT_TYPE_REL_WITHIN = 10;
const int SPLIT_TYPE_REL_WITHIN_EQUAL = 11;
const int SPLIT_TYPE_REL_WITHOUT = 12;
const int SPLIT_TYPE_REL_WITHOUT_EQUAL = 13;

bool is_match_helper(std::vector<double>& obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range);

#endif /* DECISION_TREE_H */