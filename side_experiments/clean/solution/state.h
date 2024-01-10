#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractNode;
class StateNetwork;
class Scope;

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	State();
	State(std::ifstream& input_file,
		  int id,
		  std::string path,
		  std::string name);
	~State();

	void save(std::ofstream& output_file,
			  std::string path,
			  std::string name);
};

#endif /* STATE_H */