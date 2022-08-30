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

	std::vector<Layer*> potential_inputs;	// each size 1
	std::vector<Layer*> potential_hiddens;	// each size 4

	int epoch;
	int iter;

	std::mutex mtx;

	Network(int input_size,
			int hidden_size,
			int output_size);
	Network(Network* original);
	Network(std::ifstream& input_file);
	~Network();

	void activate(std::vector<double>& vals);
	void activate(std::vector<double>& vals,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void backprop_errors_with_no_weight_change(std::vector<double>& errors);

	void add_potential();
	void activate(std::vector<double>& vals,
				  std::vector<int>& potentials_on,
				  std::vector<double>& potential_vals);
	void activate(std::vector<double>& vals,
				  std::vector<int>& potentials_on,
				  std::vector<double>& potential_vals,
				  std::vector<NetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors,
				  std::vector<int>& potentials_on);
	void extend_with_potential(int potential_index);
	void delete_potential(int potential_index);
	void pad_input();
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

	std::vector<int> potentials_on;
	std::vector<double> potential_inputs_historys;
	std::vector<std::vector<double>> potential_hiddens_historys;

	NetworkHistory(Network* network);
	NetworkHistory(Network* network, std::vector<int> potentials_on);
	~NetworkHistory();
	void reset_weights();
};

#endif /* NETWORK_H */