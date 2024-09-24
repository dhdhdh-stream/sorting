#ifndef NN_HELPERS_H
#define NN_HELPERS_H

class DecisionNetwork;
class EvalNetwork;
class StateNetwork;

void train_rl(DecisionNetwork* decision_network,
			  StateNetwork* state_network,
			  EvalNetwork* eval_network,
			  double& average_val);

#endif /* NN_HELPERS_H */