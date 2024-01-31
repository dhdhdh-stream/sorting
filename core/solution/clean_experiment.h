/**
 * TODO:
 * - remove CleanExperiment
 *   - too easy to destroy progress
 *     - instead rely on score to restrict solution size
 *   - also make PassThroughExperiment recursive to prevent it from destroying progress
 *     - i.e., from destroy progress misguess success + branch success
 */

#ifndef CLEAN_EXPERIMENT_H
#define CLEAN_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;

const int CLEAN_EXPERIMENT_STATE_MEASURE_EXISTING = 0;
const int CLEAN_EXPERIMENT_STATE_MEASURE_NEW = 1;
const int CLEAN_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 2;
const int CLEAN_EXPERIMENT_STATE_VERIFY_1ST = 3;
const int CLEAN_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 4;
const int CLEAN_EXPERIMENT_STATE_VERIFY_2ND = 5;
/**
 * - even though 3 times >0.0 is unlikely, also makes false positives less likely
 */

const int CLEAN_EXPERIMENT_STATE_FAIL = 6;
const int CLEAN_EXPERIMENT_STATE_SUCCESS = 7;

class CleanExperimentOverallHistory;
class CleanExperiment : public AbstractExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	int state;
	int state_iter;

	double existing_score;

	int clean_exit_depth;
	AbstractNode* clean_exit_node;

	double new_score;

	CleanExperiment(std::vector<int> scope_context,
					std::vector<int> node_context);
	~CleanExperiment();

	void activate(AbstractNode*& curr_node,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper,
				  CleanExperimentOverallHistory* history);

	void measure_existing_backprop(double target_val,
								   RunHelper& run_helper);

	void measure_new_initial_activate(AbstractNode*& curr_node,
									  std::vector<ContextLayer>& context,
									  int& exit_depth,
									  AbstractNode*& exit_node);
	void measure_new_activate(AbstractNode*& curr_node,
							  int& exit_depth,
							  AbstractNode*& exit_node);
	void measure_new_backprop(double target_val);

	void verify_existing_backprop(double target_val,
								  RunHelper& run_helper);

	void verify_new_activate(AbstractNode*& curr_node,
							 int& exit_depth,
							 AbstractNode*& exit_node);
	void verify_new_backprop(double target_val);
};

class CleanExperimentOverallHistory : public AbstractExperimentHistory {
public:
	CleanExperimentOverallHistory(CleanExperiment* experiment);
};

#endif /* CLEAN_EXPERIMENT_H */