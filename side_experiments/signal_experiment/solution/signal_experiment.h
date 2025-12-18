/**
 * - train on explore
 *   - OK if bad average due to experiment's train existing
 * 
 * - don't worry about pre vs. post as some scopes will be evaluated fully after explore
 */

#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <fstream>
#include <vector>

#include "network.h"

class SignalExperiment {
public:
	Network* network;

	std::vector<double> signal_history;

	SignalExperiment();
	SignalExperiment(std::ifstream& input_file);
	~SignalExperiment();

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_EXPERIMENT_H */