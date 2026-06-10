#ifndef PREDICT_WRAPPER_H
#define PREDICT_WRAPPER_H

#include <fstream>
#include <vector>

class Network;
class StateNetwork;

const int NUM_PREDICT = 5;

class PredictWrapper {
public:
	std::vector<StateNetwork*> val_networks;
	std::vector<int> val_epoch_iters;
	std::vector<double> val_average_max_updates;

	std::vector<Network*> select_networks;

	double misguess_average;

	PredictWrapper();
	PredictWrapper(PredictWrapper* original);
	PredictWrapper(std::ifstream& input_file);
	~PredictWrapper();

	void add_states();

	void twiddle();

	void save(std::ofstream& output_file);
};

#endif /* PREDICT_WRAPPER_H */