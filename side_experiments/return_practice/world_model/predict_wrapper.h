#ifndef PREDICT_WRAPPER_H
#define PREDICT_WRAPPER_H

#include <fstream>
#include <vector>

class Network;
class StateNetwork;
class Wrapper;

const int NUM_PREDICT = 4;

class PredictWrapper {
public:
	StateNetwork* average_network;
	int average_epoch_iter;
	double average_average_max_update;

	std::vector<StateNetwork*> val_networks;
	std::vector<int> val_epoch_iters;
	std::vector<double> val_average_max_updates;

	std::vector<Network*> select_networks;

	double misguess_average;

	PredictWrapper(Wrapper* wrapper);
	PredictWrapper(PredictWrapper* original);
	PredictWrapper(std::ifstream& input_file);
	~PredictWrapper();

	void add_states(int num_add);

	void twiddle();

	void save(std::ofstream& output_file);
};

#endif /* PREDICT_WRAPPER_H */