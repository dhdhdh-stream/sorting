#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class ActionNetwork;
class Sample;
class StateNetwork;

void train_network(std::vector<Sample*>& samples,
				   StateNetwork* state_network,
				   ActionNetwork* action_network);

#endif /* NN_HELPERS_H */