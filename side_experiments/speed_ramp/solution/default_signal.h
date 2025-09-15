#ifndef DEFAULT_SIGNAL_H
#define DEFAULT_SIGNAL_H

#include <fstream>
#include <vector>

class SignalNetwork;

class DefaultSignal {
public:
	std::vector<bool> score_input_is_pre;
	std::vector<int> score_input_indexes;
	std::vector<int> score_input_obs_indexes;
	SignalNetwork* score_network;

	DefaultSignal();
	DefaultSignal(std::ifstream& input_file);
	~DefaultSignal();

	double calc(std::vector<std::vector<double>>& pre_obs_histories,
				std::vector<std::vector<double>>& post_obs_histories);

	void save(std::ofstream& output_file);
};

#endif /* DEFAULT_SIGNAL_H */