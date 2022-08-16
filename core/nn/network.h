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
	Layer* hidden;
	Layer* output;

	std::vector<Layer*> potential_inputs;
	std::vector<Layer*> potential_hiddens;

	int epoch;
	int iter;

	std::mutex mtx;

	Network(int input_size,
			int hidden_size,
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

	void add_potential();
	void activate(std::vector<double>& vals,
				  int potential_index,
				  std::vector<double>& potential_vals,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop(int potential_index,
				  std::vector<double>& errors);
	void extend_with_potential(int potential_index);
	void reset_potential(int potential_index);
	void remove_potentials();

	void save(std::ofstream& output_file);

private:
	void construct(int input_size,
				   int hidden_size,
				   int output_size);
};

class NetworkHistory {
public:
	Network* network;

	std::vector<double> input_history;
	std::vector<double> hidden_history;
	std::vector<double> output_history;

	int potential_index;
	std::vector<double> potential_input_history;
	std::vector<double> potential_hidden_history;

	NetworkHistory(Network* network);
	NetworkHistory(Network* network, int potential_index);
	~NetworkHistory();
	void reset_weights();
};

#endif /* NETWORK_H */