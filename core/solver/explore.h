#ifndef EXPLORE_H
#define EXPLORE_H

#include <mutex>
#include <vector>

#include "action.h"
#include "loop.h"
#include "network.h"
#include "solution_node.h"

const int EXPLORE_STATE_EXPLORE = 0;

const int NEW_PATH_STATE_MEASURE_AVERAGE = 1;
const int NEW_PATH_STATE_LEARN_SCORES = 2;
const int NEW_PATH_STATE_MEASURE_INFORMATION = 3;

const int LOOP_STATE_CHECK_DIFFERENT_ITERS = 4;
const int LOOP_STATE_LEARN_LOOP = 5;
const int LOOP_STATE_MEASURE_AVERAGE = 6;
const int LOOP_STATE_LEARN_SCORES = 7;
const int LOOP_STATE_MEASURE_INFORMATION = 8;

const int EXPLORE_TYPE_NEW_PATH = 0;
const int EXPLORE_TYPE_LOOP = 1;
// const int EXPLORE_TYPE_MERGE = 2;

class SolutionNode;
class Explore {
public:
	SolutionNode* parent;

	int state;
	int current_id;
	int candidate_iter;
	double best_score;	// use information for score for now
	int best_candidate_type;

	int iter_index;

	double average_score;
	double think_good_and_good;
	double think_good_but_bad;

	// NEW_PATH
	std::vector<Action> best_new_path;

	std::vector<Action> current_new_path;

	// NEW_PATH_STATE_LEARN_SCORES
	Network* new_path_learn_scores_network;

	// LOOP
	Loop* best_loop;

	Loop* current_loop;

	// LOOP_STATE_CHECK_DIFFERENT_ITERS
	int different_iters_success_counts[7];

	std::mutex mtx;

	Explore(SolutionNode* parent);
	~Explore();

	void process(Problem& p,
				 std::vector<double>& observations,
				 double& score,
				 bool save_for_display,
				 std::vector<Action>* raw_actions);
};

#endif /* EXPLORE_H */