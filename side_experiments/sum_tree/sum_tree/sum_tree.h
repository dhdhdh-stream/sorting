/**
 * - only update/split leaves
 *   - otherwise, too easy to destroy progress/naunce
 *     - especially with noise
 */

/**
 * TODO: only gather 1 sample per run(?)
 */

#ifndef SUM_TREE_H
#define SUM_TREE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class SumTreeNode;

#if defined(MDEBUG) && MDEBUG
const int ST_NUM_TRAIN_SAMPLES = 40;
const int ST_NUM_TEST_SAMPLES = 10;
#else
const int ST_NUM_TRAIN_SAMPLES = 4000;
const int ST_NUM_TEST_SAMPLES = 1000;
#endif /* MDEBUG */
const int ST_NUM_TOTAL_SAMPLES = ST_NUM_TRAIN_SAMPLES + ST_NUM_TEST_SAMPLES;

class SumTree {
public:
	int node_counter;
	std::map<int, SumTreeNode*> nodes;

	SumTreeNode* root;

	std::vector<double> improvement_history;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;
	int history_index;

	SumTree();
	~SumTree();

	double activate(std::vector<double>& obs);
	void backprop(std::vector<double>& obs,
				  double target_val);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(SumTree* original);

	void init_helper();
	void update_helper(SumTreeNode* node);
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

#endif /* SUM_TREE_H */