#ifndef TUNNEL_H
#define TUNNEL_H

#include <fstream>
#include <vector>

class Network;
class SolutionWrapper;

class Tunnel {
public:
	std::vector<int> obs_indexes;

	bool is_pattern;
	Network* similarity_network;

	Network* signal_network;

	double starting_true_average;
	double starting_true_standard_deviation;
	double starting_val_average;
	double starting_val_standard_deviation;

	double current_true_average;
	double current_true_standard_deviation;
	double current_val_average;
	double current_val_standard_deviation;

	int num_tries;
	/**
	 * TODO:
	 * - if num_train_fail low, then means signal guards against bad cases
	 */
	int num_train_fail;
	int num_measure_fail;
	int num_success;

	Tunnel(std::vector<int>& obs_indexes,
		   bool is_pattern,
		   Network* similarity_network,
		   Network* signal_network,
		   SolutionWrapper* wrapper);
	Tunnel(Tunnel* original);
	Tunnel(std::ifstream& input_file);
	~Tunnel();

	double get_signal(SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);

	void print();
};

#endif /* TUNNEL_H */