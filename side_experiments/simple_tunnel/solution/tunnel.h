#ifndef TUNNEL_H
#define TUNNEL_H

#include <fstream>
#include <vector>

class SolutionWrapper;

class Tunnel {
public:
	bool is_default;
	int obs_index;

	int num_tries;
	int num_successes;

	std::vector<double> true_improvements;
	std::vector<double> signal_improvements;

	Tunnel(bool is_default,
		   int obs_index);
	Tunnel(std::ifstream& input_file);

	double get_signal(SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
};

#endif /* TUNNEL_H */