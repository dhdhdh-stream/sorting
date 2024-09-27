#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class DecisionNetwork;
class EvalNetwork;
class StateNetwork;

void train_rl(std::vector<DecisionNetwork*>& decision_networks,
			  StateNetwork* state_network,
			  EvalNetwork* eval_network,
			  double& average_val);

#endif /* NN_HELPERS_H */