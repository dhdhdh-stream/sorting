#ifndef COMBINED_NN_HELPERS_H
#define COMBINED_NN_HELPERS_H

class CombinedDecisionNetwork;
class EvalNetwork;
class StateNetwork;

void train_rl_combined(CombinedDecisionNetwork* decision_network,
					   StateNetwork* state_network,
					   EvalNetwork* eval_network,
					   double& average_val);

#endif /* COMBINED_NN_HELPERS_H */