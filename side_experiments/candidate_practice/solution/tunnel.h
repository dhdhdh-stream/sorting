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

const int TUNNEL_TRY_STATUS_SIGNAL_FAIL = 0;
const int TUNNEL_TRY_STATUS_TRUE_FAIL = 1;
const int TUNNEL_TRY_STATUS_TRUE_SUCCESS = 2;

class Tunnel {
public:
	std::vector<int> obs_indexes;

	bool is_pattern;
	Network* similarity_network;

	Network* signal_network;

	/**
	 * - compare num_tries vs. num_train_fail
	 *   - if <50%, protects
	 *   - if >50%, misleads
	 */
	int num_tries;
	int num_train_fail;
	int num_measure_fail;
	int num_success;

	std::vector<int> try_history;

	std::vector<double> val_history;

	/**
	 * - temp for measuring val
	 */
	std::vector<double> vals;

	Tunnel(std::vector<int>& obs_indexes,
		   bool is_pattern,
		   Network* similarity_network,
		   Network* signal_network);
	Tunnel(Tunnel* original);
	Tunnel(std::ifstream& input_file);
	~Tunnel();

	double get_signal(std::vector<double>& obs);

	void update_vals(int num_runs);
	bool is_fail();
	bool is_long_term();

	void save(std::ofstream& output_file);

	void print();
};

#endif /* TUNNEL_H */