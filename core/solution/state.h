#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractNode;
class Scale;
class Scope;
class StateNetwork;

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	State();
	State(std::ifstream& input_file,
		  int id);
	~State();

	void detach(Scope* parent_scope);

	void save(std::ofstream& output_file);
};

#endif /* STATE_H */