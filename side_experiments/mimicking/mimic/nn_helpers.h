#ifndef NN_HELPERS_H
#define NN_HELPERS_H

#include <vector>

class ActionNetwork;
class LSTM;
class Sample;

void train_network(std::vector<Sample*>& samples,
				   std::vector<LSTM*>& memory_cells,
				   ActionNetwork* action_network);

#endif /* NN_HELPERS_H */