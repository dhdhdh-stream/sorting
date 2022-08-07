#ifndef NETWORK_H
#define NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "layer.h"

class NetworkHistory;
class Network {
public:
	Layer* input;
	Layer* val_1st;
	Layer* val_val;

	int epoch;
	int iter;

	std::mutex mtx;

	Network(int input_size,
			int layer_size,
			int output_size);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& vals);
	void activate(std::vector<double>& vals,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors);
	void increment();
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void save(std::ofstream& output_file);

private:
	void construct(int input_size,
				   int layer_size,
				   int output_size);
};

class NetworkHistory {
public:
	Network* network;

	std::vector<double> input_history;
	std::vector<double> val_1st_history;
	std::vector<double> val_val_history;

	NetworkHistory(Network* network);
	~NetworkHistory();
	void reset_weights();
};

#endif /* NETWORK_H */