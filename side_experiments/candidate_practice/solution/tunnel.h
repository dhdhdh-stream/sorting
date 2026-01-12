/**
 * - don't worry about generalization
 *   - can generalize in multiple directions, so no simple answer
 *   - instead, constantly look for new ones
 * 
 * - can't simply identify when a tunnel is maxed or even reversed
 *   - correlation with target from explore samples can remain similar
 */

#ifndef TUNNEL_H
#define TUNNEL_H

#include <fstream>
#include <vector>

class Network;
class Scope;
class Solution;
class SolutionWrapper;

class Tunnel {
public:
	std::vector<int> obs_indexes;

	bool is_pattern;
	Network* similarity_network;

	Network* signal_network;

	int num_tries;
	int num_significant;
	int num_improve;

	Tunnel(std::vector<int>& obs_indexes,
		   bool is_pattern,
		   Network* similarity_network,
		   Network* signal_network);
	Tunnel(Tunnel* original);
	Tunnel(std::ifstream& input_file);
	~Tunnel();

	double get_signal(std::vector<double>& obs);

	void save(std::ofstream& output_file);

	void print();
};

#endif /* TUNNEL_H */