#ifndef FACTOR_H
#define FACTOR_H

#include <fstream>
#include <utility>
#include <vector>

class Network;
class Scope;
class ScopeHistory;

class Factor {
public:
	bool is_used;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> inputs;
	Network* network;

	Factor();
	~Factor();

	double back_activate(ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
};

#endif /* FACTOR_H */