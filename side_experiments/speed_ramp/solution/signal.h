/**
 * - reward signal is as much about preserving existing, as making improvements
 *   - as signal only valid under conditions
 * 
 * - there can be multiple options for "consistency" that make sense
 *   - e.g., train at a train station
 *     - could be correct to follow train or follow train station
 * - the option that should be chosen is the one that produces the best signal
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <fstream>
#include <vector>

class SignalNetwork;

class Signal {
public:
	std::vector<bool> match_input_is_pre;
	std::vector<int> match_input_indexes;
	std::vector<int> match_input_obs_indexes;
	SignalNetwork* match_network;

	std::vector<bool> score_input_is_pre;
	std::vector<int> score_input_indexes;
	std::vector<int> score_input_obs_indexes;
	SignalNetwork* score_network;

	Signal();
	Signal(Signal* original);
	Signal(std::ifstream& input_file);
	~Signal();

	void calc(std::vector<std::vector<double>>& pre_obs_histories,
			  std::vector<std::vector<double>>& post_obs_histories,
			  bool& is_match,
			  double& val);

	void insert(bool is_pre,
				int index,
				int exit_index,
				int length);

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_H */