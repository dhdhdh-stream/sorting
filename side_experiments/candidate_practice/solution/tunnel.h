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

	double starting_true;
	double ending_true;

	int num_tries;
	int num_train_fail;
	int num_measure_fail;
	int num_success;

	Tunnel(std::vector<int>& obs_indexes,
		   bool is_pattern,
		   Network* similarity_network,
		   Network* signal_network,
		   SolutionWrapper* wrapper);
	Tunnel(std::ifstream& input_file);
	~Tunnel();

	double get_signal(SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);

	void print();
};

#endif /* TUNNEL_H */