/**
 * - first, using target path:
 *   - find score function
 *   - find nodes that have lower variance than average
 *     - taking into account correlation as well
 * - second, on random paths, evaluate:
 *   - if fixed points match, does score function apply
 */

#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

#include <vector>

#include "input.h"

class AbstractNode;
class Network;
class Problem;
class Scope;

const int EVAL_EXPERIMENT_TRAIN = 0;
const int EVAL_EXPERIMENT_MEASURE = 1;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class EvalExperiment {
public:
	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	int state;
	int state_iter;

	std::vector<AbstractNode*> nodes;

	std::vector<Input> inputs;

	double score_average_val;
	double score_standard_deviation;

	double obs_average_val;
	double obs_standard_deviation;

	std::vector<Input> fixed_points;
	std::vector<double> fixed_point_average_vals;
	std::vector<double> fixed_point_standard_deviations;

	std::vector<Input> score_inputs;
	Network* score_network;

	int result;

	std::vector<std::vector<double>> input_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> predicted_score_histories;

	EvalExperiment(Scope* scope_context);
	~EvalExperiment();

	void activate(Problem* problem);
	void backprop(double target_val);

	void train_activate(Problem* problem);
	void train_backprop(double target_val);

	void measure_activate(Problem* problem);
	void measure_backprop(double target_val);
};

#endif /* EVAL_EXPERIMENT_H */