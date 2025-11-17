// TODO: add replace experiment

#ifndef LOGIC_EXPERIMENT_H
#define LOGIC_EXPERIMENT_H

#include <vector>

class AbstractLogicNode;
class LogicTree;
class Network;

const int LOGIC_EXPERIMENT_STATE_GATHER = 0;
const int LOGIC_EXPERIMENT_STATE_MEASURE = 1;
const int LOGIC_EXPERIMENT_STATE_DONE = 2;

class LogicExperiment {
public:
	AbstractLogicNode* node_context;

	int state;
	int state_iter;

	Network* split_network;
	Network* eval_network;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	double sum_improvement;
	int count;

	double improvement;

	LogicExperiment(AbstractLogicNode* node_context);
	~LogicExperiment();

	void activate(std::vector<double>& obs,
				  double target_val);

	void gather_activate(std::vector<double>& obs,
						 double target_val);

	void measure_activate(std::vector<double>& obs,
						  double target_val);

	void add(LogicTree* logic_tree);
};

#endif /* LOGIC_EXPERIMENT_H */