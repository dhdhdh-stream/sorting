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

const int TRY_FAIL = 0;
const int TRY_IMPROVE = 1;

class Tunnel {
public:
	std::vector<int> obs_indexes;

	bool is_pattern;
	Network* similarity_network;

	Network* signal_network;

	std::vector<int> try_history;

	std::vector<std::vector<double>> starting_best_obs;
	std::vector<std::vector<double>> starting_worst_obs;
	std::vector<std::vector<double>> starting_random_obs;

	std::vector<std::vector<double>> latest_existing_obs;
	std::vector<std::vector<double>> latest_new_obs;

	Tunnel(std::vector<int>& obs_indexes,
		   bool is_pattern,
		   Network* similarity_network,
		   Network* signal_network);
	Tunnel(Tunnel* original);
	Tunnel(std::ifstream& input_file);
	~Tunnel();

	double get_signal(std::vector<double>& obs);

	bool is_fail();

	void save(std::ofstream& output_file);

	void print();

	void print_obs();
};

#endif /* TUNNEL_H */