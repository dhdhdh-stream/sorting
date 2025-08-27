#ifndef SIGNAL_INSTANCE_H
#define SIGNAL_INSTANCE_H

#include <fstream>
#include <vector>

class SignalNetwork;

class SignalInstance {
public:
	std::vector<bool> match_input_is_pre;
	std::vector<int> match_input_indexes;
	std::vector<int> match_input_obs_indexes;
	SignalNetwork* match_network;

	std::vector<bool> score_input_is_pre;
	std::vector<int> score_input_indexes;
	std::vector<int> score_input_obs_indexes;
	SignalNetwork* score_network;

	SignalInstance();
	SignalInstance(SignalInstance* original);
	SignalInstance(std::ifstream& input_file);
	~SignalInstance();

	void insert(bool is_pre,
				int index,
				int exit_index,
				int length);

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_INSTANCE_H */