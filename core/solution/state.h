#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractNode;
class Scale;
class StateNetwork;

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	std::set<int> resolved_network_indexes;
	double resolved_standard_deviation;

	Scale* scale;

	std::vector<AbstractNode*> nodes;
	/**
	 * - for tracking while still score state
	 * 
	 * - may be duplicates if multiples obs from same node
	 */

	State();
	State(std::ifstream& input_file,
		  int id);
	~State();

	void detach();

	void save(std::ofstream& output_file);
};

#endif /* STATE_H */