#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class ActionNetwork;
class LSTM;
class Sample;

void train_network(std::vector<Sample*>& samples,
				   std::vector<LSTM*>& memory_cells,
				   std::vector<ActionNetwork*>& action_networks);

#endif /* NN_HELPERS_H */